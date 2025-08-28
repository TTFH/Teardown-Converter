#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "scene.h"
#include "vox_writer.h"
#include "xml_writer.h"
#include "zlib_utils.h"

using namespace std;

const char* td_notes[32] = {
	"snow/hole",
	"reserved",
	"unphysical",
	"unphysical",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"ice",
	"hardmasonry",
	"hardmetal",
	"plastic",
	"plastic",
	"heavymetal",
	"heavymetal",
	"metal",
	"metal",
	"plaster",
	"plaster",
	"masonry",
	"masonry",
	"masonry",
	"masonry",
	"wood",
	"wood",
	"rock",
	"rock",
	"dirt",
	"dirt",
	"foliage",
	"foliage",
	"glass"
};

const char* MatOverridePrefix = "$TD_";

bool MVShape::operator==(const MVShape& other) const {
	if (voxels.sizex != other.voxels.sizex || voxels.sizey != other.voxels.sizey || voxels.sizez != other.voxels.sizez)
		return false;
	for (int x = 0; x < voxels.sizex; x++)
		for (int y = 0; y < voxels.sizey; y++)
			for (int z = 0; z < voxels.sizez; z++)
				if (voxels.Get(x, y, z) != other.voxels.Get(x, y, z))
					return false;
	return true;
}

MV_FILE::MV_FILE(string filename) {
	this->filename = filename;
	vox_file = NULL;

	palette[0] = { 0, 0, 0, 255 };
	for (int i = 1; i < 256; i++)
		palette[i] = { 75, 75, 75, 255};

	for (int i = 0; i < 256; i++) {
		is_index_used[i] = false;
		palette_map[i] = i;
	}
}

void MV_FILE::WriteInt(int val) {
	fwrite(&val, sizeof(int), 1, vox_file);
}

void MV_FILE::WriteDICT(DICT dict) {
	WriteInt(dict.size());
	for (DICT::iterator it = dict.begin(); it != dict.end(); it++) {
		WriteInt(it->first.length());
		fwrite(it->first.c_str(), it->first.length(), sizeof(char), vox_file);
		WriteInt(it->second.length());
		fwrite(it->second.c_str(), it->second.length(), sizeof(char), vox_file);
	}
}

void MV_FILE::WriteChunkHeader(int id, int contentSize, int childrenSize) {
	WriteInt(id);
	WriteInt(contentSize);
	WriteInt(childrenSize);
}

void MV_FILE::WriteFileHeader() {
	WriteInt(VOX);
	WriteInt(VERSION);
	WriteInt(MAIN);
	WriteInt(0); // contentSize
	childrenSize_ptr = ftell(vox_file);
	WriteInt(0); // Main chunk childrens size, will be updated later
}

void MV_FILE::WriteSIZE(MVShape shape) {
	WriteChunkHeader(SIZE, 12, 0);
	WriteInt(shape.voxels.sizex);
	WriteInt(shape.voxels.sizey);
	WriteInt(shape.voxels.sizez);
}

void MV_FILE::WriteXYZI(MVShape shape) {
	int voxel_count = shape.voxels.GetNonZeroCount();
	WriteChunkHeader(XYZI, 4 * (1 + voxel_count), 0);
	WriteInt(voxel_count);

	for (int x = 0; x < shape.voxels.sizex; x++) {
		for (int y = 0; y < shape.voxels.sizey; y++) {
			for (int z = 0; z < shape.voxels.sizez; z++) {
				uint8_t index = shape.voxels.Get(x, y, z);
				if (index != 0) {
					Voxel voxel = { (uint8_t)x, (uint8_t)y, (uint8_t)z, index };
					fwrite(&voxel, sizeof(Voxel), 1, vox_file);
				}
			}
		}
	}
}

void MV_FILE::WriteTDCZ(MVShape shape) {
	uint8_t* voxel_array = shape.voxels.ToArray();
	// TODO: dynamic allocation
	int compressed_size = shape.voxels.GetVolume() + shape.voxels.GetNonZeroCount() + 10;
	uint8_t* compressed_data = new uint8_t[compressed_size];
	ZlibBlockCompress(voxel_array, shape.voxels.GetVolume(), 9, compressed_data, compressed_size);

	WriteChunkHeader(TDCZ, 3 * sizeof(int) + compressed_size, 0);
	WriteInt(shape.voxels.sizex);
	WriteInt(shape.voxels.sizey);
	WriteInt(shape.voxels.sizez);
	fwrite(compressed_data, sizeof(uint8_t), compressed_size, vox_file);

	delete[] voxel_array;
	delete[] compressed_data;
}

void MV_FILE::WriteMain_nTRN() {
	int num_models = models.size();
	WriteChunkHeader(nTRN, 28, 0);
	WriteInt(0); // node_id
	WriteInt(0); // Empty DICT (nodeAttribs)
	WriteInt(1); // child_node_id
	WriteInt(-1); // reserved_id
	WriteInt(-1); // layer_id
	WriteInt(1); // num_frames
	WriteInt(0); // Empty DICT (frames)

	WriteChunkHeader(nGRP, 4 * (3 + num_models), 0);
	WriteInt(1); // node_id
	WriteInt(0); // node_attr
	WriteInt(num_models); // node_childrens
	for (int i = num_models; i > 0; i--)
		WriteInt(2 * i); // child_node

	for (int i = 1; i <= num_models; i++) {
		string pos = to_string(models[i-1].pos_x) + " " + to_string(models[i-1].pos_y) + " " + to_string(models[i-1].pos_z);

		WriteChunkHeader(nTRN, 38 + pos.length() + 13 + models[i-1].name.length(), 0);
		WriteInt(2 * i); // node_id

		DICT node_attr;
		node_attr["_name"] = models[i-1].name;
		WriteDICT(node_attr);

		WriteInt(2 * i + 1); // child_node_id
		WriteInt(-1); // reserved_id
		WriteInt(0); // layer_id
		WriteInt(1); // num_frames

		DICT frame_attr;
		frame_attr["_t"] = pos;
		WriteDICT(frame_attr);

		WriteChunkHeader(nSHP, 20, 0);
		WriteInt(2 * i + 1); // node_id
		WriteInt(0); // node_attr
		WriteInt(1); // num_models
		WriteInt(i - 1); // ref_model_id
		WriteInt(0); // Empty DICT (nodeAttribs)
	}
}

void MV_FILE::WriteRGBA() {
	WriteChunkHeader(RGBA, 1024, 0);
	fwrite(&palette[1], sizeof(MV_Entry), 255, vox_file);
	fwrite(&palette[0], sizeof(MV_Entry), 1, vox_file);
}

bool MV_FILE::IsSnow(uint8_t index, uint8_t type) {
	return type == Material::Unphysical &&
			palette[palette_map[index]].r == 229 &&
			palette[palette_map[index]].g == 229 &&
			palette[palette_map[index]].b == 229;
}

bool MV_FILE::IsHole(uint8_t index, uint8_t type) {
	return type == Material::Unphysical &&
			palette[palette_map[index]].r == 255 &&
			palette[palette_map[index]].g == 0 &&
			palette[palette_map[index]].b == 0;
}

static bool IsInRange(uint8_t index, uint8_t i_min, uint8_t i_max) {
	return index >= i_min && index <= i_max;
}

static bool IsIndexCorrupted(uint8_t index, uint8_t type) {
	bool corrupted = false;
	if (type == Material::Glass)
		corrupted = !IsInRange(index, 1, 8);
	else if (type == Material::Foliage)
		corrupted = !IsInRange(index, 9, 24);
	else if (type == Material::Dirt)
		corrupted = !IsInRange(index, 25, 40);
	else if (type == Material::Rock)
		corrupted = !IsInRange(index, 41, 56);
	else if (type == Material::Wood)
		corrupted = !IsInRange(index, 57, 72);
	else if (type == Material::Masonry)
		corrupted = !IsInRange(index, 73, 104);
	else if (type == Material::Plaster)
		corrupted = !IsInRange(index, 105, 120);
	else if (type == Material::Metal)
		corrupted = !IsInRange(index, 121, 136);
	else if (type == Material::HeavyMetal)
		corrupted = !IsInRange(index, 137, 152);
	else if (type == Material::Plastic)
		corrupted = !IsInRange(index, 153, 168);
	else if (type == Material::HardMetal)
		corrupted = !IsInRange(index, 169, 176);
	else if (type == Material::HardMasonry)
		corrupted = !IsInRange(index, 177, 184);
	else if (type == Material::Ice)
		corrupted = !IsInRange(index, 185, 192);
	else if ( type == Material::None)
		corrupted = !IsInRange(index, 193, 224) && !IsInRange(index, 241, 253);
	else if (type == Material::Unphysical)
		corrupted = !IsInRange(index, 225, 240) && index != 254 && index != 255;
	return corrupted;
}

bool MV_FILE::FixMapping(uint8_t index, uint8_t i_min, uint8_t i_max, bool halt) {
	unsigned int mapped_index = 1;
	while (palette_map[mapped_index] != index && mapped_index < 256)
		mapped_index++;
	assert(mapped_index < 256);

	// Only move if not in range, or it'll break vehicle lights
	if (mapped_index < i_min || mapped_index > i_max) {
		unsigned int empty_index = i_min;
		while (empty_index <= i_max && is_index_used[empty_index])
			empty_index++;

		if (empty_index <= i_max) {
			is_index_used[empty_index] = true;
			is_index_used[mapped_index] = false;

			uint8_t temp = palette_map[mapped_index];
			palette_map[mapped_index] = palette_map[empty_index];
			palette_map[empty_index] = temp;
		} else {
			if (!halt && FixMapping(index, 193, 224, true))
				return true;
			if (!halt && FixMapping(index, 241, 253, true))
				return true;
			return false;
		}
	}
	return true;
}

static const int FIX_ATTEMPTS = 3;

void MV_FILE::WriteIMAP() {
	// Try to set every material to its correct index
	for (int i = 0; i < FIX_ATTEMPTS; i++)
	for (vector<MV_Material>::iterator it = materials.begin(); it != materials.end(); it++) {
		if (it->material_type == Material::Glass)
			FixMapping(it->material_index, 1, 8);
		else if (it->material_type == Material::Foliage)
			FixMapping(it->material_index, 9, 24);
		else if (it->material_type == Material::Dirt)
			FixMapping(it->material_index, 25, 40);
		else if (it->material_type == Material::Rock)
			FixMapping(it->material_index, 41, 56);
		else if (it->material_type == Material::Wood)
			FixMapping(it->material_index, 57, 72);
		else if (it->material_type == Material::Masonry)
			FixMapping(it->material_index, 73, 104);
		else if (it->material_type == Material::Plaster)
			FixMapping(it->material_index, 105, 120);
		else if (it->material_type == Material::Metal)
			FixMapping(it->material_index, 121, 136);
		else if (it->material_type == Material::HeavyMetal)
			FixMapping(it->material_index, 137, 152);
		else if (it->material_type == Material::Plastic)
			FixMapping(it->material_index, 153, 168);
		else if (it->material_type == Material::HardMetal)
			FixMapping(it->material_index, 169, 176);
		else if (it->material_type == Material::HardMasonry)
			FixMapping(it->material_index, 177, 184);
		else if (it->material_type == Material::Ice)
			FixMapping(it->material_index, 185, 192);
		else if (it->material_type == Material::None)
			FixMapping(it->material_index, 193, 224);
		else if (IsSnow(it->material_index, it->material_type))
			FixMapping(it->material_index, 254, 254);
		else if (IsHole(it->material_index, it->material_type))
			FixMapping(it->material_index, 255, 255);
		else // Unphysical
			FixMapping(it->material_index, 225, 240);
	}

	// Check for material overflow
	uint8_t palette_reverse_map[256];
	for (int i = 0; i < 256; i++)
		palette_reverse_map[palette_map[i]] = i;

	for (vector<MV_Material>::iterator it = materials.begin(); it != materials.end(); it++) {
		uint8_t index = palette_reverse_map[it->material_index];
		if (IsIndexCorrupted(index, it->material_type)) {
			is_corrupted = true;
			break;
		}
	}

	// If nothing changed, don't write IMAP
	bool is_mapped = false;
	for (int i = 0; i < 256; i++)
		if (palette_map[i] != i)
			is_mapped = true;
	if (!is_mapped) return;

	WriteChunkHeader(IMAP, 256, 0);
	fwrite(&palette_map[1], sizeof(uint8_t), 255, vox_file);
	fwrite(&palette_map[0], sizeof(uint8_t), 1, vox_file);
}

void MV_FILE::WriteMATL(MV_Material mat) {
	DICT material_attr;
	if (mat.render_type == METAL) {
		material_attr["_type"] = "_metal";
		material_attr["_rough"] = FloatToString(mat.properties.metal.roughness);
		material_attr["_sp"] = FloatToString(mat.properties.metal.specular);
		material_attr["_metal"] = FloatToString(mat.properties.metal.metallic);
	} else if (mat.render_type == GLASS) {
		material_attr["_type"] = "_glass";
		material_attr["_rough"] = FloatToString(mat.properties.glass.roughness);
		material_attr["_alpha"] = "0.5";
	} else if (mat.render_type == EMIT) {
		material_attr["_type"] = "_emit";
		material_attr["_emit"] = FloatToString(mat.properties.emit.emission);
		material_attr["_flux"] = to_string(mat.properties.emit.power);
	} else return;
	int matl_size = 8 + 8 * material_attr.size();
	for (DICT::iterator it = material_attr.begin(); it != material_attr.end(); it++)
		matl_size += it->first.length() + it->second.length();

	WriteChunkHeader(MATL, matl_size, 0);
	WriteInt(mat.material_index);
	WriteDICT(material_attr);
}

void MV_FILE::WriteNOTE() {
	vector<string> notes;
	notes.reserve(32);
	bool repaired = true;
	if (is_corrupted) {
		for (int row = 1; row <= 32; row++) {
			string note = "";
			bool error = false;
			bool row_corrupted = false;
			uint8_t row_material = 255; // uninitialized

			for (int col = 1; col <= 8; col++) {
				unsigned int index = 8 * (32 - row) + col;
				if (index < 256 && is_index_used[index]) {
					uint8_t material = 255; // uninitialized
					for (vector<MV_Material>::iterator it = materials.begin(); it != materials.end(); it++)
						if (it->material_index == palette_map[index]) {
							material = it->material_type;
							break;
						}
					assert(material != 255);

					if (IsIndexCorrupted(index, material)) {
						row_corrupted = true;
						if (row_material == 255)
							row_material = material; // Set material for row
						if (row_material != material) {
							error = true;
							repaired = false;
							break;
						}
					}
				}
			}
			if (error) // Row has multiple materials
				note = "corrupted";
			else if (row_corrupted) // Row has a single material, but it's in the wrong place because overflow
				note = string(MatOverridePrefix) + MaterialName[row_material];
			else // Row is fine
				note = td_notes[row - 1];
			notes.push_back(note);
		}
	} else
		for (int i = 0; i < 32; i++)
			notes.push_back(td_notes[i]);
	if (!repaired)
		printf("Corrupted row(s) in file %s\n", filename.c_str());

	int note_chunck_size = 4 + 4 * 32;
	for (int i = 0; i < 32; i++)
		note_chunck_size += notes[i].length();

	WriteChunkHeader(NOTE, note_chunck_size, 0);
	WriteInt(32);
	for (int i = 0; i < 32; i++) {
		WriteInt(notes[i].length());
		fwrite(notes[i].c_str(), sizeof(char), notes[i].length(), vox_file);
	}
}

void MV_FILE::SaveModel(bool compress) {
	vox_file = fopen(filename.c_str(), "wb+");
	if (vox_file == NULL) {
		printf("[ERROR] Could not open %s for writing\n", filename.c_str());
		return;
	}

	WriteFileHeader();
	for (unsigned int i = 0; i < models.size(); i++) {
		WriteSIZE(models[i]);
		if (compress)
			WriteTDCZ(models[i]);
		else
			WriteXYZI(models[i]);
	}
	WriteMain_nTRN();
	WriteRGBA();
	WriteIMAP();
	for (vector<MV_Material>::iterator it = materials.begin(); it != materials.end(); it++)
		WriteMATL(*it);
	WriteNOTE();

	// Update childrenSize in the MAIN chunk
	long int size = ftell(vox_file);
	fseek(vox_file, childrenSize_ptr, SEEK_SET);
	WriteInt(size - childrenSize_ptr - sizeof(int)); // Minus 4 bytes of childrenSize
	fclose(vox_file);
}

void MV_FILE::AddShape(MVShape shape) {
	models.push_back(shape);
}

bool MV_FILE::GetShapeName(const MVShape& shape, string& name) const {
	bool found = false;
	for (vector<MVShape>::const_iterator it = models.begin(); it != models.end() && !found; it++)
		if (*it == shape) {
			found = true;
			name = it->name;
		}
	return found;
}

void MV_FILE::SetColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
	if (is_index_used[index]) return;
	palette[index] = { r, g, b, 255};
}

/*
_type = "_metal"
reflectivity = _sp - 1.0
shinyness = 1.0 - _rough
metalness = _metal
emissive = 0.0
alpha = 1.0

_type = "_glass"
reflectivity = 0.1
shinyness = 1.0 - _rough
metalness = 0.0
emissive = 0.0
alpha = _alpha != 1.0 ? 0.5 : 1.0

_type = "_emit"
reflectivity = 0.1
shinyness = 1.0
metalness = 0.0
emissive = _emit * 10 ^ _flux
alpha = 1.0
*/
void MV_FILE::SetMaterial(uint8_t index, uint8_t type, float reflectivity, float shinyness, float metalness, float emissive, float alpha) {
	if (is_index_used[index]) return;
	MV_Material mat;
	mat.material_index = index;
	mat.material_type = type;

	if (alpha != 1) {
		mat.render_type = GLASS;
		mat.properties.glass.roughness = 1.0 - shinyness;
	} else if (emissive > 0) {
		mat.render_type = EMIT;
		int flux = 0;
		if (emissive > 100.0)
			flux = 4;
		else if (emissive > 10.0)
			flux = 3;
		else if (emissive > 1.0)
			flux = 2;
		else if (emissive > 0.1)
			flux = 1;
		mat.properties.emit.emission = emissive / pow(10, flux - 1);
		mat.properties.emit.power = flux;
	} else if (reflectivity > 0 || shinyness > 0 || metalness > 0) {
		mat.render_type = METAL;
		mat.properties.metal.roughness = 1.0 - shinyness;
		mat.properties.metal.specular = 1.0 + reflectivity;
		mat.properties.metal.metallic = metalness;
	} else
		mat.render_type = DIFFUSE;
	materials.push_back(mat);

	is_index_used[index] = true;
}

MV_FILE::~MV_FILE() {
	for (vector<MVShape>::iterator it = models.begin(); it != models.end(); it++)
		it->voxels.Clear();
}
