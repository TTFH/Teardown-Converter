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

const int VERSION = 200;
const int VOX  = ID('V', 'O', 'X', ' ');
const int MAIN = ID('M', 'A', 'I', 'N');
const int SIZE = ID('S', 'I', 'Z', 'E');
const int XYZI = ID('X', 'Y', 'Z', 'I');
const int TDCZ = ID('T', 'D', 'C', 'Z');
const int RGBA = ID('R', 'G', 'B', 'A');
const int nGRP = ID('n', 'G', 'R', 'P');
const int nTRN = ID('n', 'T', 'R', 'N');
const int nSHP = ID('n', 'S', 'H', 'P');
const int MATL = ID('M', 'A', 'T', 'L');
const int IMAP = ID('I', 'M', 'A', 'P');
const int NOTE = ID('N', 'O', 'T', 'E');

enum MV_MaterialType {
	DIFFUSE,
	METAL,
	GLASS,
	EMIT
};

struct MV_Material {
	uint8_t td_type;
	MV_MaterialType type;
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

struct MV_Color {
	uint8_t r, g, b, a;
};

struct MV_Voxel {
	uint8_t x, y, z, index;
};

struct MV_Shape {
	string name;
	int pos_x, pos_y, pos_z;
	Tensor3D voxels;
	bool operator==(const MV_Shape& other) const;
};

class MV_FILE {
private:
	FILE* vox_file;
	string filename;
	bool write_imap;
	long int children_size_ptr;
	vector<MV_Shape> models;
	static const int ROWS = 32;
	string notes[ROWS];

	bool is_index_used[256];
	MV_Color palette[256];
	MV_Material material[256];
	uint8_t palette_map[256];

	string GetIndexNote(int index);
	void FIX_PALETTE_MAPPING();

	void WriteInt(int val);
	void WriteDICT(DICT dict);
	void WriteFileHeader();
	void WriteChunkHeader(int id, int content_size, int children_size);

	void WriteSIZE(const MV_Shape& shape);
	void WriteXYZI(const MV_Shape& shape);
	void WriteTDCZ(const MV_Shape& shape);
	void Write_nGRP();
	void Write_nTRN(int i, string pos);
	void Write_nSHP(int i);
	void WriteRGBA();
	void WriteIMAP();
	void WriteMATL(uint8_t index, const MV_Material& mat);
	void WriteNOTE();
public:
	MV_FILE(string filename, bool write_imap = true);
	void SaveModel(bool compress = false);
	void AddShape(const MV_Shape& shape);
	bool GetShapeName(const MV_Shape& shape, string& name) const;
	void SetEntry(uint8_t index, const MV_Color& color, MV_Material mat);
};

const int SNOW_INDEX = 254;
const MV_Color SNOW_COLOR = { 230, 230, 230, 255 };
const MV_Material SNOW_MATERIAL = { Material::Unphysical, METAL, { 0.8f, 1.1f, 0.0f } };

const uint8_t HOLE_INDEX = 255;
const MV_Color HOLE_COLOR = { 255, 0, 0, 255 };
const MV_Material HOLE_MATERIAL = { Material::None, GLASS, { 1.0f } };

#endif
