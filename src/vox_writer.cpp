#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "scene.h"
#include "vox_writer.h"

using namespace std;

const char* notes[] = {
	"snow / hole",
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

static string FloatToString(float value) {
	if (fabs(value) < 0.001) value = 0;
	stringstream ss;
	ss << fixed << setprecision(3) << value;
	string str = ss.str();
	if (str.find('.') != string::npos) {
		str = str.substr(0, str.find_last_not_of('0') + 1);
		if (str.find('.') == str.size() - 1)
			str = str.substr(0, str.size() - 1);
	}
	return str;
}

uint8_t*** MatrixInit(int sizex, int sizey, int sizez) {
	uint8_t*** matrix = new uint8_t**[sizex];
	for (int i = 0; i < sizex; i++) {
		matrix[i] = new uint8_t*[sizey];
		for (int j = 0; j < sizey; j++)
			matrix[i][j] = new uint8_t[sizez];
	}
	for (int i = 0; i < sizex; i++)
		for (int j = 0; j < sizey; j++)
			for (int k = 0; k < sizez; k++)
				matrix[i][j][k] = 0x00;
	return matrix;
}

void MatrixDelete(uint8_t*** &matrix, int sizex, int sizey) {
	for (int i = 0; i < sizex; i++) {
		for (int j = 0; j < sizey; j++)
			delete[] matrix[i][j];
		delete[] matrix[i];
	}
	delete[] matrix;
	matrix = NULL;
}

bool MVShape::operator==(const MVShape& other) const {
	if (sizex != other.sizex || sizey != other.sizey || sizez != other.sizez)
		return false;
	for (int x = 0; x < sizex; x++)
		for (int y = 0; y < sizey; y++)
			for (int z = 0; z < sizez; z++)
				if (voxels[x][y][z] != other.voxels[x][y][z])
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
	is_index_used[0] = true;
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
	WriteInt(shape.sizex);
	WriteInt(shape.sizey);
	WriteInt(shape.sizez);
}

void MV_FILE::WriteXYZI(MVShape shape) {
	int voxel_count = 0;
	for (int x = 0; x < shape.sizex; x++)
		for (int y = 0; y < shape.sizey; y++)
			for (int z = 0; z < shape.sizez; z++)
				if (shape.voxels[x][y][z] != 0)
					voxel_count++;

	WriteChunkHeader(XYZI, 4 * (1 + voxel_count), 0);
	WriteInt(voxel_count);

	for (int x = 0; x < shape.sizex; x++) {
		for (int y = 0; y < shape.sizey; y++) {
			for (int z = 0; z < shape.sizez; z++) {
				uint8_t index = shape.voxels[x][y][z];
				if (index != 0) {
					Voxel voxel = { (uint8_t)x, (uint8_t)y, (uint8_t)z, index };
					fwrite(&voxel, sizeof(Voxel), 1, vox_file);
				}
			}
		}
	}
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
	fwrite(&palette[1], sizeof(Color), 255, vox_file);
	fwrite(&palette[0], sizeof(Color), 1, vox_file);
}

bool MV_FILE::FixMapping(uint8_t index, uint8_t i_min, uint8_t i_max) {
	unsigned int mapped_index = 1;
	while (palette_map[mapped_index] != index && mapped_index < 256)
		mapped_index++;
	assert(mapped_index < 256);

	if (mapped_index < i_min || mapped_index > i_max) {
		unsigned int empty_index = i_min;
		while (is_index_used[empty_index] && empty_index <= i_max)
			empty_index++;

		if (empty_index <= i_max) {
			is_index_used[empty_index] = true;
			is_index_used[mapped_index] = false;
			uint8_t temp = palette_map[mapped_index];
			palette_map[mapped_index] = palette_map[empty_index];
			palette_map[empty_index] = temp;
		} else {
			if (FixMapping(mapped_index, 193, 224))
				return true;
			if (FixMapping(mapped_index, 241, 253))
				return true;
			return false;
		}
	}
	return true;
}

bool MV_FILE::CheckMaterial(uint8_t index, uint8_t i_min, uint8_t i_max) {
	return index >= i_min && index <= i_max;
}

void MV_FILE::WriteIMAP() {
	for (int i = 0; i < 2; i++)
	for (vector<PBR>::iterator it = pbrs.begin(); it != pbrs.end(); it++) {
		bool is_snow = it->material_type == MaterialKind::Unphysical &&
			palette[palette_map[it->material_index]].r == 229 &&
			palette[palette_map[it->material_index]].g == 229 &&
			palette[palette_map[it->material_index]].b == 229;

		if (it->material_type == MaterialKind::Glass)
			FixMapping(it->material_index, 1, 8);
		else if (it->material_type == MaterialKind::Foliage)
			FixMapping(it->material_index, 9, 24);
		else if (it->material_type == MaterialKind::Dirt)
			FixMapping(it->material_index, 25, 40);
		else if (it->material_type == MaterialKind::Rock)
			FixMapping(it->material_index, 41, 56);
		else if (it->material_type == MaterialKind::Wood)
			FixMapping(it->material_index, 57, 72);
		else if (it->material_type == MaterialKind::Masonry)
			FixMapping(it->material_index, 73, 104);
		else if (it->material_type == MaterialKind::Plaster)
			FixMapping(it->material_index, 105, 120);
		else if (it->material_type == MaterialKind::Metal)
			FixMapping(it->material_index, 121, 136);
		else if (it->material_type == MaterialKind::HeavyMetal)
			FixMapping(it->material_index, 137, 152);
		else if (it->material_type == MaterialKind::Plastic)
			FixMapping(it->material_index, 153, 168);
		else if (it->material_type == MaterialKind::HardMetal)
			FixMapping(it->material_index, 169, 176);
		else if (it->material_type == MaterialKind::HardMasonry)
			FixMapping(it->material_index, 177, 184);
		else if (it->material_type == MaterialKind::Ice)
			FixMapping(it->material_index, 185, 192);
		else if ( it->material_type == MaterialKind::None)
			FixMapping(it->material_index, 193, 224);
		else if (!is_snow)
			FixMapping(it->material_index, 225, 240);
		else // Snow
			FixMapping(it->material_index, 254, 254);
	}

	uint8_t palette_reverse_map[256];
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			if (palette_map[i] == j)
				palette_reverse_map[j] = i;

	bool corrupted = false;
	for (vector<PBR>::iterator it = pbrs.begin(); it != pbrs.end() && !corrupted; it++) {
		uint8_t index = palette_reverse_map[it->material_index];
		if (it->material_type == MaterialKind::Glass)
			corrupted = !CheckMaterial(index, 1, 8);
		else if (it->material_type == MaterialKind::Foliage)
			corrupted = !CheckMaterial(index, 9, 24);
		else if (it->material_type == MaterialKind::Dirt)
			corrupted = !CheckMaterial(index, 25, 40);
		else if (it->material_type == MaterialKind::Rock)
			corrupted = !CheckMaterial(index, 41, 56);
		else if (it->material_type == MaterialKind::Wood)
			corrupted = !CheckMaterial(index, 57, 72);
		else if (it->material_type == MaterialKind::Masonry)
			corrupted = !CheckMaterial(index, 73, 104);
		else if (it->material_type == MaterialKind::Plaster)
			corrupted = !CheckMaterial(index, 105, 120);
		else if (it->material_type == MaterialKind::Metal)
			corrupted = !CheckMaterial(index, 121, 136);
		else if (it->material_type == MaterialKind::HeavyMetal)
			corrupted = !CheckMaterial(index, 137, 152);
		else if (it->material_type == MaterialKind::Plastic)
			corrupted = !CheckMaterial(index, 153, 168);
		else if (it->material_type == MaterialKind::HardMetal)
			corrupted = !CheckMaterial(index, 169, 176);
		else if (it->material_type == MaterialKind::HardMasonry)
			corrupted = !CheckMaterial(index, 177, 184);
		else if (it->material_type == MaterialKind::Ice)
			corrupted = !CheckMaterial(index, 185, 192);
		else if ( it->material_type == MaterialKind::None)
			corrupted = !CheckMaterial(index, 193, 224) && !CheckMaterial(index, 241, 253);
		else if (it->material_type == MaterialKind::Unphysical)
			corrupted = !CheckMaterial(index, 225, 240) && !CheckMaterial(index, 254, 255);	
		if (corrupted)
			printf("Overflow of %s at pallete: %s.\n", MaterialKindName[it->material_type], filename.c_str());
	}

	WriteChunkHeader(IMAP, 256, 0);
	fwrite(&palette_map[1], sizeof(uint8_t), 255, vox_file);
	fwrite(&palette_map[0], sizeof(uint8_t), 1, vox_file);
}

void MV_FILE::WriteMATL(PBR pbr) {
	DICT material_attr;
	material_attr["_type"] = pbr.type;
	if (pbr.type == "_metal") {
		material_attr["_rough"] = FloatToString(pbr.rough);
		material_attr["_metal"] = FloatToString(pbr.metal);
	} else if (pbr.type == "_glass") {
		material_attr["_rough"] = FloatToString(pbr.rough);
		material_attr["_alpha"] = FloatToString(pbr.alpha);
	} else if (pbr.type == "_emit") {
		material_attr["_emit"] = FloatToString(pbr.emit);
		material_attr["_flux"] = to_string(pbr.flux);
	}
	int matl_size = 8 + 8 * material_attr.size();
	for (DICT::iterator it = material_attr.begin(); it != material_attr.end(); it++)
		matl_size += it->first.length() + it->second.length();

	WriteChunkHeader(MATL, matl_size, 0);
	WriteInt(pbr.material_index);
	WriteDICT(material_attr);
}

void MV_FILE::WriteNOTE() {
	int note_count = sizeof(notes) / sizeof(notes[0]);;
	int note_chunck_size = 4 + 4 * note_count;
	for (int i = 0; i < note_count; i++)
		note_chunck_size += strlen(notes[i]);

	WriteChunkHeader(NOTE, note_chunck_size, 0);
	WriteInt(note_count);
	for (int i = 0; i < note_count; i++) {
		WriteInt(strlen(notes[i]));
		fwrite(notes[i], sizeof(char), strlen(notes[i]), vox_file);
	}
}

void MV_FILE::SaveModel() {
	vox_file = fopen(filename.c_str(), "wb+");
	if (vox_file == NULL) {
		printf("Error: Could not open %s for writing\n", filename.c_str());
		exit(EXIT_FAILURE);
	}

	WriteFileHeader();
	for (unsigned int i = 0; i < models.size(); i++) {
		WriteSIZE(models[i]);
		WriteXYZI(models[i]);
	}
	WriteMain_nTRN();
	WriteRGBA();
	WriteIMAP();
	for (vector<PBR>::iterator it = pbrs.begin(); it != pbrs.end(); it++)
		WriteMATL(*it);
	WriteNOTE();

	// Update childrenSize in the MAIN chunk
	long int size = ftell(vox_file);
	fseek(vox_file, childrenSize_ptr, SEEK_SET);
	WriteInt(size - childrenSize_ptr - sizeof(int)); // Minus 4 bytes of childrenSize
}

void MV_FILE::AddColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
	palette[index] = { r, g, b, 255};
	is_index_used[index] = true;
}

void MV_FILE::AddPBR(uint8_t index, uint8_t type, float reflectivity, float shinyness, float metalness, float emissive, float alpha) {
	PBR pbr;
	pbr.material_index = index;
	pbr.material_type = type;

	if (alpha != 0 && alpha != 1) {
		pbr.type = "_glass";
		pbr.rough = reflectivity;
		pbr.alpha = alpha;
	} else if (emissive > 0) {
		pbr.type = "_emit";
		if (emissive > 100)
			pbr.flux = 4;
		else if (emissive > 10)
			pbr.flux = 3;
		else if (emissive > 1)
			pbr.flux = 2;
		else
			pbr.flux = 1;
		pbr.emit = emissive / pow(10, pbr.flux - 1);
	} else if (shinyness > 0) {
		pbr.type = "_metal";
		pbr.rough = 1.0 - shinyness;
		pbr.metal = metalness != 1 ? metalness : 0;
	} else
		pbr.type = "_diffuse";

	pbrs.push_back(pbr);
}

MV_FILE::~MV_FILE() {
	for (vector<MVShape>::iterator it = models.begin(); it != models.end(); it++)
		MatrixDelete(it->voxels, it->sizex, it->sizey);
	if (vox_file != NULL)
		fclose(vox_file);
}
