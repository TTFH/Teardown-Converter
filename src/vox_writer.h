#ifndef VOX_WRITER_H
#define VOX_WRITER_H

#include <stdio.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

using namespace std;

typedef map<string, string> DICT;

constexpr int ID(char a, char b, char c, char d) {
	return a | (b << 8) | (c << 16) | (d << 24);
}

const int VERSION = 150;
const int VOX  = ID('V', 'O', 'X', ' ');
const int MAIN = ID('M', 'A', 'I', 'N');
const int SIZE = ID('S', 'I', 'Z', 'E');
const int XYZI = ID('X', 'Y', 'Z', 'I');
const int TDCZ = ID('T', 'D', 'C', 'Z');
const int RGBA = ID('R', 'G', 'B', 'A');
const int nTRN = ID('n', 'T', 'R', 'N');
const int nSHP = ID('n', 'S', 'H', 'P');

const int IMAP = ID('I', 'M', 'A', 'P');
const int MATL = ID('M', 'A', 'T', 'L');
const int nGRP = ID('n', 'G', 'R', 'P');
const int LAYR = ID('L', 'A', 'Y', 'R');
const int rOBJ = ID('r', 'O', 'B', 'J');
const int rCAM = ID('r', 'C', 'A', 'M');
const int NOTE = ID('N', 'O', 'T', 'E');

struct MV_Entry {
	uint8_t r, g, b, a;
};

enum MaterialType {
	DIFFUSE,
	METAL,
	GLASS,
	EMIT
};

struct MV_Material {
	uint8_t material_index;
	uint8_t material_type;
	MaterialType render_type;
	union {
		struct {
			float roughness;
			float specular;
			float metallic;
		} metal;
		struct {
			float roughness;
		} glass;
		struct {
			float emission;
			int power;
		} emit;
	} properties;
};

struct Voxel {
	uint8_t x, y, z, index;
};

struct MVShape {
	string name;
	int pos_x, pos_y, pos_z;
	Tensor3D voxels;
	bool operator==(const MVShape& other) const;
};

class MV_FILE {
private:
	FILE* vox_file;
	string filename;
	MV_Entry palette[256];
	uint8_t palette_map[256];
	bool is_index_used[256];
	bool is_corrupted = false;
	vector<MVShape> models;
	vector<MV_Material> materials;
	long int childrenSize_ptr;

	bool IsSnow(uint8_t index, uint8_t type);
	bool IsHole(uint8_t index, uint8_t type);
	bool FixMapping(uint8_t index, uint8_t i_min, uint8_t i_max, bool halt = false);

	void WriteInt(int val) {
		fwrite(&val, sizeof(int), 1, vox_file);
	}
	void WriteDICT(DICT dict);
	void WriteFileHeader();
	void WriteChunkHeader(int id, int contentSize, int childrenSize);
	void WriteSIZE(MVShape shape);
	void WriteXYZI(MVShape shape);
	void WriteTDCZ(MVShape shape);
	void WriteMain_nTRN();
	void WriteRGBA();
	void WriteIMAP();
	void WriteMATL(MV_Material mat);
	void WriteNOTE();
public:
	MV_FILE(string filename);
	~MV_FILE();
	void SaveModel(bool compress);
	void AddShape(MVShape shape);
	bool GetShapeName(const MVShape& shape, string& name);
	void SetColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
	void SetMaterial(uint8_t index, uint8_t kind, float reflectivity, float shinyness, float metalness, float emissive, float alpha);
};

#endif
