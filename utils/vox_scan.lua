-- Save one or multiple shapes with the same palette to a Magica Voxel file

function init()
	g_shape = 0 -- shape id
	model_id = 0 -- model id to be stored in savegame.xml
	palette = {} -- RGBA data
	models = {} -- SIZE and XYZI data

	invalid = false -- true if an error ocurred
	shape_data = {} -- material, color and position for each voxel of the selected shape
	data_index = 1 -- index in the list above
	process_data = false -- true if there is remaining data to process for a shape
	THREADS = 10 -- How many voxels are processed in each call to tick()
end

-- Inserts a RGB color into the palette, between the index i_min and i_max inclusive
-- Return the index asigned to that color, or 0 if that section of the palette is full
function InsertColor(r, g, b, i_min, i_max)
	local index = i_min
	local found = false
	while not found and index <= i_max do
		if palette[index] == nil then
			palette[index] = {r, g, b}
			found = true
		else
			local p = palette[index]
			if p[1] == r and p[2] == g and p[3] == b then
				found = true
			else
				index = index + 1
			end
		end
	end
	if index > i_max then
		index = 0
	end
	return index
end

-- Inserts a RGB color into the palette, in one of the index asociated to the specified material
-- Return the index asigned to that color, or 0 if the section of the palette asigned to that material is full
-- Shapes with more that 16 color of wood will overflow to the reserved area
function GetIndex(mat, nr, ng, nb)
	local r = math.floor(nr * 255)
	local g = math.floor(ng * 255)
	local b = math.floor(nb * 255)

	local index = 0
	if mat == "glass" then
		index = InsertColor(r, g, b, 1, 8)
	elseif mat == "foliage" then
		index = InsertColor(r, g, b, 9, 24)
	elseif mat == "dirt" then
		index = InsertColor(r, g, b, 25, 40)
	elseif mat == "rock" then
		index = InsertColor(r, g, b, 41, 56)
	elseif mat == "wood" then
		index = InsertColor(r, g, b, 57, 72)
		if index == 0 then
			index = InsertColor(r, g, b, 193, 224)
		end
		if index == 0 then
			index = InsertColor(r, g, b, 241, 253)
		end
		if index == 0 then
			DebugPrint("THE FUCK?") -- Compound has too many different wood colours
		end
	elseif mat == "masonry" then
		index = InsertColor(r, g, b, 73, 104)
	elseif mat == "plaster" then
		index = InsertColor(r, g, b, 105, 120)
	elseif mat == "metal" then
		index = InsertColor(r, g, b, 121, 136)
	elseif mat == "heavymetal" then
		index = InsertColor(r, g, b, 137, 152)
	elseif mat == "plastic" then
		index = InsertColor(r, g, b, 153, 168)
	elseif mat == "hardmetal" then
		index = InsertColor(r, g, b, 169, 176)
	elseif mat == "hardmasonry" then
		index = InsertColor(r, g, b, 177, 184)
	elseif mat == "ice" then
		index = InsertColor(r, g, b, 185, 192)
	elseif mat == "none" then
		index = InsertColor(r, g, b, 193, 224)
		if index == 0 then
			index = InsertColor(r, g, b, 241, 253)
		end
	elseif mat == "unphysical" then
		if (r == 229 and g == 229 and b == 229) then
			index = InsertColor(r, g, b, 254, 254) -- snow
		else
			index = InsertColor(r, g, b, 225, 240)
		end
	else
		DebugPrint("ERROR! Unsupported material " .. mat)
	end
	if index == 0 then
		DebugPrint("ERROR! Too many colors for material " .. mat)
	end
	return index
end

-- Round a number to the closest integer
function round(x)
	return math.floor(x + 0.5)
end

-- Converts a byte into a 2 characters string
function ToHex(num)
	return string.format("%02X", num)
end

-- Convert an integer to an 8 characters string
function ToInt32(num)
	return string.format("%08X", ToLittleEndian(num))
end

-- Save a XYZI entry into an 8 characters string
function HexVoxel(x, y, z, index)
	return ToHex(x) .. ToHex(y) .. ToHex(z) .. ToHex(index)
end

-- Converts a number to litthe endian, it may work for negative numbers too
function ToLittleEndian(num)
	local res = 0
	for i = 1, 4 do
		res = res * 256 + num % 256
		num = math.floor(num / 256)
	end
	return res
end

-- Generates a MV SIZE chunk with the given size
function GenerateSIZE(xsize, ysize, zsize)
	local size = "53495A45" .. "0C000000" .. "00000000"
	size = size .. ToInt32(xsize) .. ToInt32(ysize) .. ToInt32(zsize)
	return size
end

-- Generates a MV RGBA chunk from the palette, unused entries has the color (75, 75, 75)
function GenerateRGBA()
	local rgba = "52474241" .. "00040000" .. "00000000"
	for i = 1, 255 do
		if palette[i] ~= nil then
			rgba = rgba .. ToHex(palette[i][1]) .. ToHex(palette[i][2]) .. ToHex(palette[i][3]) .. "FF"
		else
			rgba = rgba .. "4B4B4BFF"
		end
	end
	rgba = rgba .. "00000000"
	return rgba
end

function ScanShapeInit(shape)
	local xsize, ysize, zsize = GetShapeSize(shape)
	if xsize > 256 or ysize > 256 or zsize > 256 then
		DebugPrint("ERROR! Shape too big")
		g_shape = 0
		return
	end

	local voxel_count = GetShapeVoxelCount(shape)
	local eta = math.floor(voxel_count / (60 * THREADS))
	DebugPrint("ETA " .. math.floor(eta / 60) .. " min " .. eta % 60 .. " seg")

	local contentSize = ToInt32(4 * voxel_count + 4)
	local numVoxels = ToInt32(voxel_count)
	local xyzi = "58595A49" .. contentSize .. "00000000" .. numVoxels

	-- Clear shape data
	shape_data = {}
	data_index = 1

	for z = 0, zsize - 1 do
		for y = 0, ysize - 1 do
			for x = 0, xsize - 1 do
				local mat, r, g, b = GetShapeMaterialAtIndex(shape, x, y, z)
				if mat ~= "" then
					table.insert(shape_data, {mat = mat, x = x, y = y, z = z, r = r, g = g, b = b})
				end
			end
		end
	end
	process_data = true

	local size = GenerateSIZE(xsize, ysize, zsize)
	table.insert(models, {size = size, xyzi = xyzi})
end

-- Generates Magica Voxel metadata for multiple shapes
-- https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox-extension.txt
function GenerateMetadata(num_models)
	local main_ntrn = ""
	if num_models > 1 then
		main_ntrn = "6E54524E" .. ToInt32(28) .. "00000000"
		main_ntrn = main_ntrn .. "00000000" .. "00000000" .. "01000000" .. "FFFFFFFF" .. "FFFFFFFF"
		main_ntrn = main_ntrn .. "01000000" .. "00000000"

		local ngrp_size = 12 + 4 * num_models
		local ngrp = "6E475250" .. ToInt32(ngrp_size) .. "00000000"
		ngrp = ngrp .. "01000000" .. "00000000" .. ToInt32(num_models)
		for i = num_models, 1, -1 do
			ngrp = ngrp .. ToInt32(2 * i)
		end
		main_ntrn = main_ntrn .. ngrp

		for i = 1, num_models do
			local ntrn = "6E54524E" .. ToInt32(28) .. "00000000"
			ntrn = ntrn .. ToInt32(2 * i) .. "00000000" .. ToInt32(2 * i + 1) .. "FFFFFFFF" .. "00000000"
			ntrn = ntrn .. "01000000" .. "00000000"
			main_ntrn = main_ntrn .. ntrn

			local nshp = "6E534850" .. ToInt32(20) .. "00000000"
			nshp = nshp .. ToInt32(2 * i + 1) .. "00000000" .. "01000000" .. ToInt32(i - 1) .. "00000000"
			main_ntrn = main_ntrn .. nshp
		end
	end
	return main_ntrn
end

-- Save all the scanned shapes to savegame.xml
function SaveModel()
	local main_ntrn = GenerateMetadata(#models)
	local rgba = GenerateRGBA()
	local contentSize = string.len(main_ntrn) + string.len(rgba)
	for i = 1, #models do
		contentSize = contentSize + string.len(models[i].size) + string.len(models[i].xyzi)
	end
	contentSize = math.floor(contentSize / 2)

	local model = "564F5820" .. "96000000" .. "4D41494E" .. "00000000" .. ToInt32(contentSize)
	for i = 1, #models do
		model = model .. models[i].size .. models[i].xyzi
	end
	model = model .. main_ntrn .. rgba
	SetString("savegame.mod.VOX" .. model_id, model)

	models = {}
	palette = {} -- Clear palette
	model_id = model_id + 1
end

-- Process a single voxel
function ScanShapeTick()
	if process_data and not invalid then
		local data = shape_data[data_index]
		if data ~= nil then
			local index = GetIndex(data.mat, data.r, data.g, data.b)
			if index == 0 then
				--invalid = true
				g_shape = 0
				process_data = false
				table.remove(models)
				return
			end
			DebugWatch("Processing", data.mat .. " " .. index)
			local voxel = HexVoxel(data.x, data.y, data.z, index)
			local entry = models[#models] -- Reference to current model
			entry.xyzi = entry.xyzi .. voxel
			data_index = data_index + 1
		else
			process_data = false
			DebugPrint("Done!")
			DebugPrint("[press F5 to save models and clear palette]")
			DebugPrint("[or scan more shapes to be stored in the same file]")
			g_shape = 0
		end
	end
end

function tick(dt)
	if InputPressed("mmb") and g_shape == 0 then
		local camera_tr = GetCameraTransform()
		local camera_fwd = TransformToParentVec(camera_tr, Vec(0, 0, -1))
		--QueryRequire("physical large")
		local hit, _, _, shape = QueryRaycast(camera_tr.pos, camera_fwd, 10)
		if hit then
			ScanShapeInit(shape)
			g_shape = shape
		end
	end
	if g_shape ~= 0 then
		for i = 1, THREADS do
			ScanShapeTick()
		end
		DrawShapeOutline(g_shape, 1, 0, 0, 0.5)
	else
		if InputPressed("F5") then
			SaveModel()
			DebugPrint("Model saved!")
			DebugPrint("Palette cleared!")
		end
	end
end
