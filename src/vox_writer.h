#ifndef VOX_WRITER_H
#define VOX_WRITER_H

#include <stdint.h>
#include <stdio.h>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

using namespace std;

typedef map<string, string> DICT;

uint8_t*** MatrixInit(int sizex, int sizey, int sizez);
void MatrixDelete(uint8_t*** &matrix, int sizex, int sizey);

constexpr int ID(char a, char b, char c, char d) {
	return a | (b << 8) | (c << 16) | (d << 24);
}

const int VERSION = 150;
const int VOX  = ID('V', 'O', 'X', ' ');
const int MAIN = ID('M', 'A', 'I', 'N');
const int SIZE_c = ID('S', 'I', 'Z', 'E');
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

struct Color {
	uint8_t r, g, b, a;
};

struct PBR {
	int material_index;
	uint8_t material_type;
	string type; // _diffuse, _metal, _glass, _emit
	float rough;
	float metal;
	float emit;
	int flux; // [0..4]
	float alpha;
};

struct Voxel {
	uint8_t x, y, z, index;
};

struct MVShape {
	int sizex, sizey, sizez;
	uint8_t*** voxels;
	string name;
	int pos_x, pos_y, pos_z;

	bool operator==(const MVShape& other) const;
};

class MV_FILE {
private:
	FILE* vox_file;
	string filename;
	vector<PBR> pbrs;
	Color palette[256];
	uint8_t palette_map[256];
	long int childrenSize_ptr;

	void WriteInt(int val) {
		fwrite(&val, sizeof(int), 1, vox_file);
	}
	void WriteDICT(DICT dict);
	void WriteFileHeader();
	void WriteChunkHeader(int id, int contentSize, int childrenSize);
	void WriteSIZE(MVShape shape);
	void WriteXYZI(MVShape shape);
	void WriteMain_nTRN();
	void WriteRGBA();
	bool FixMapping(uint8_t index, uint8_t i_min, uint8_t i_max);
	bool CheckMaterial(uint8_t index, uint8_t i_min, uint8_t i_max);
	void WriteIMAP();
	void WriteMATL(PBR pbr);
	void WriteNOTE();
public:
	bool is_index_used[256];
	vector<MVShape> models;

	MV_FILE(string filename);
	void AddShape(MVShape shape) {
		models.push_back(shape);
	}
	void SaveModel();
	void AddColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
	void AddPBR(uint8_t index, uint8_t type, float reflectivity, float shinyness, float metalness, float emissive, float alpha);
	~MV_FILE();
};

#endif
