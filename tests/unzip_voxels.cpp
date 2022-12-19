#include <stdio.h>
#include <stdint.h>

struct Voxels {
	uint32_t size[3];
	// if the shape volume is not empty, voxels are stored using run length encoding
	// with pairs (n-1, i) in xyz order
	uint8_t* palette_index;
};

uint8_t ReadByte(FILE* file) {
	uint8_t byte = 0;
	fread(&byte, sizeof(uint8_t), 1, file);
	return byte;
}

uint32_t ReadInt(FILE* file) {
	uint32_t val = 0;
	fread(&val, sizeof(uint32_t), 1, file);
	return val;
}

int main() {
	FILE* file = fopen("rle.bin", "rb");
	Voxels voxels;
	for (int i = 0; i < 3; i++)
		voxels.size[i] = ReadInt(file);

	int volume = voxels.size[0] * voxels.size[1] * voxels.size[2];
	if (volume > 0) {
		int encoded_length = ReadInt(file);
		voxels.palette_index = new uint8_t[volume];
		int k = 0;
		for (int i = 0; i < encoded_length / 2; i++) {
			int run_length = ReadByte(file);
			uint8_t voxel_index = ReadByte(file);
			for (int j = 0; j <= run_length; j++) {
				voxels.palette_index[k] = voxel_index;
				k++;
			}
		}
	}
	fclose(file);

	FILE* out_file = fopen("voxels.bin", "wb");
	fwrite(voxels.palette_index, sizeof(uint8_t), volume, out_file);
	fclose(out_file);
	return 0;
}

/*
void MV_FILE::WriteNOTE() {
	vector<string> notes;
	notes.reserve(32);
	for (int row = 1; row <= 32; row++) {
		string note = "";
		bool error = false;
		bool row_corrupted = false;
		uint8_t row_material = 255; // uninitialized
		for (int col = 1; col <= 8; col++) {
			uint8_t index = 8 * (32 - row) + col;
			if (IsIndexCorrupted(index)) {
				row_corrupted = true;
				if (row_material == 255)
					row_material = materials[index].mat_type;
				if (row_material != materials[index].mat_type) {
					error = true;
					note = "corrupted";
					printf("[ERROR] Corrupted row %d with different materials [%s, %s, etc.] in file %s\n", 32 - row + 1, MaterialKindName[row_material], MaterialKindName[materials[index].mat_type], filename.c_str());
				}
			}
		}
		if (row_corrupted && !error) {
			note = string(MatOverridePrefix) + MaterialKindName[row_material];
			printf("[INFO] Fixed corrupted row %d with material %s in file %s\n", 32 - row + 1, MaterialKindName[row_material], filename.c_str());
		}
		notes.push_back(note);
	}

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
*/
