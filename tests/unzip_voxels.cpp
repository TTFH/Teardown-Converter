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
