#include <stdio.h>
#include <stdint.h>

#include <SDL.h>

#include "src/parser.h"

using namespace std;
float progress = 0;

class Spraycan : public TDBIN {
private:
	FILE* out_file;

public:
	Spraycan(const char* bin_path) : TDBIN(bin_path) {
		out_file = NULL;
	}

	void hijackTintTable() {
		printf("Hijacking tint table...\n");
		// TODO: !
	}

	void WriteByte(uint8_t byte) {
		fwrite(&byte, sizeof(uint8_t), 1, out_file);
	}
	void WriteInt(uint32_t integer) {
		fwrite(&integer, sizeof(uint32_t), 1, out_file);
	}
	void WriteFloat(float flt) {
		fwrite(&flt, sizeof(float), 1, out_file);
	}
	void WriteString(string str) {
		fwrite(str.c_str(), sizeof(char), str.length() + 1, out_file);
	}

	void save(const char* filename) {
		out_file = fopen(filename, "wb");
		for (int i = 0; i < 5; i++)
			WriteByte(scene.magic[i]);
		for (int i = 0; i < 3; i++)
			WriteByte(scene.version[i]);
		WriteString(scene.level);
		WriteInt(scene.driven_vehicle);





		fclose(out_file);
	}
};

/*
229 178 25 (original)
0.9 0.7 0.1
66 66 66 3F 33 33 33 3F CD CC CC 3D 00 00 80 3F

25 178 229 (cyan)
0.1 0.7 0.9
CD CC CC 3D 33 33 33 3F 66 66 66 3F 00 00 80 3F

229 25 25 (red)
0.9 0.1 0.1
66 66 66 3F CD CC CC 3D CD CC CC 3D 00 00 80 3F
*/

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s <bin_path>\n", argv[0]);
		return 1;
	}

	Spraycan parser(argv[1]);
	parser.parse();
	parser.hijackTintTable();
	parser.save("quicksave.tdbin");

	return 0;
}
