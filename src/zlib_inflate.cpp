#include <stdio.h>
#include <zlib.h>

#include "zlib_inflate.h"

bool ZlibUncompress(const uint8_t* source, long int source_len, uint8_t* dest, long int &dest_len) {
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	stream.next_in = (uint8_t*)source;
	stream.avail_in = source_len;
	stream.next_out = dest;
	stream.avail_out = dest_len;
	dest_len = 0;

	int err = inflateInit(&stream);
	if (err != Z_OK) return false;
	err = inflate(&stream, Z_NO_FLUSH);
	inflateEnd(&stream);
	dest_len = stream.total_out;
	return err == Z_OK;
}

void UncompressFile(const char* input_file, const char* output_file) {
	FILE* bin_file = fopen(input_file, "rb");

	fseek(bin_file, 0, SEEK_END);
	long int compressed_size = ftell(bin_file);
	rewind(bin_file);

	uint8_t* compressed_data = new uint8_t[compressed_size];
	fread(compressed_data, sizeof(uint8_t), compressed_size, bin_file);
	fclose(bin_file);

	long int uncompressed_size = 2 << 27;
	uint8_t* uncompressed_data = new uint8_t[uncompressed_size];
	if (ZlibUncompress(compressed_data, compressed_size, uncompressed_data, uncompressed_size)) {
		FILE* tdbin_file = fopen(output_file, "wb");
		if (tdbin_file != NULL) {
			fwrite(uncompressed_data, sizeof(uint8_t), uncompressed_size, tdbin_file);
			fclose(tdbin_file);
		}
	}

	delete[] compressed_data;
	delete[] uncompressed_data;
}

bool IsFileCompressed(const char* filename) {
	FILE* test_file = fopen(filename, "rb");
	if (test_file == NULL) return false;
	uint8_t header[5];
	fread(header, sizeof(uint8_t), 5, test_file);
	fclose(test_file);
	return !(header[0] == 'T' && header[1] == 'D' && header[2] == 'B' && header[3] == 'I' && header[4] == 'N');
}
