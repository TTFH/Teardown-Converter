file = GetString("file", "heightmap.png", "script png")
heightScale = GetInt("scale", 256)
tileSize = GetInt("tilesize", 128)
hollow = GetInt("hollow", 0)

function init()
	matRock = CreateMaterial("rock", 0.41, 0.4, 0.37, 1, 0, 0)
	matDirt = CreateMaterial("dirt", 0.34, 0.3, 0.25, 1, 0, 0.1)
	matGrass1 = CreateMaterial("unphysical", 0.28, 0.39, 0.22, 1, 0, 0.2)
	matGrass2 = CreateMaterial("unphysical", 0.27, 0.35, 0.23, 1, 0, 0.2)
	matTarmac = CreateMaterial("masonry", 0.39, 0.39, 0.39, 1, 0, 0.4)
	matTarmacTrack = CreateMaterial("masonry", 0.3, 0.3, 0.3, 1, 0, 0.3)
	matTarmacLine = CreateMaterial("masonry", 0.7, 0.7, 0.7, 1, 0, 0.6)

	LoadImage(file)

	w, h = GetImageSize()

	local maxSize = tileSize

	local y0 = 0
	while y0 < h - 1 do
		local y1 = y0 + maxSize
		if y1 > h - 1 then y1 = h - 1 end

		local x0 = 0
		while x0 < w - 1 do
			local x1 = x0 + maxSize
			if x1 > w - 1 then x1 = w - 1 end
			Vox(x0, 0, y0)
			Heightmap(x0, y0, x1, y1, heightScale, hollow == 0)
			x0 = x1
		end
		y0 = y1
	end
end
