#include <stdio.h>
#include <string.h>

#include <zlib.h>

#include "zlib_utils.h"

static const size_t BLOCK_SIZE = 8 * 1024; // 8 KiB
static const size_t CHUNK_SIZE = 32 * 1024; // 32 KiB

bool ZlibBlockCompress(const uint8_t* source, size_t source_len, int level, vector<uint8_t>& dest) {
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.next_in = (uint8_t*)source;
	stream.avail_in = 0;

	if (deflateInit(&stream, level) != Z_OK)
		return false;

	int ret;
	dest.clear();
	dest.reserve(source_len);
	uint8_t buffer[CHUNK_SIZE];

	do {
		if (stream.avail_in == 0) {
			stream.avail_in = min<int>(source_len, BLOCK_SIZE);
			source_len -= stream.avail_in;
		}
		stream.next_out = buffer;
		stream.avail_out = CHUNK_SIZE;

		ret = deflate(&stream, Z_SYNC_FLUSH);
		size_t have = CHUNK_SIZE - stream.avail_out;
		dest.insert(dest.end(), buffer, buffer + have);

		if (source_len == 0)
			ret = Z_STREAM_END;
	} while (ret != Z_STREAM_END);

	deflateEnd(&stream);
	return true;
}

bool ZlibUncompress(const uint8_t* source, const size_t source_len, vector<uint8_t>& dest) {
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree  = Z_NULL;
	stream.opaque = Z_NULL;
	stream.next_in = (Bytef*)source;
	stream.avail_in = source_len;

	if (inflateInit(&stream) != Z_OK)
		return false;

	int ret;
	dest.clear();
	dest.reserve(source_len);
	uint8_t buffer[CHUNK_SIZE];

	do {
		stream.next_out = buffer;
		stream.avail_out = CHUNK_SIZE;

		ret = inflate(&stream, Z_NO_FLUSH);
		size_t have = CHUNK_SIZE - stream.avail_out;
		dest.insert(dest.end(), buffer, buffer + have);

		if (stream.avail_in == 0)
			ret = Z_STREAM_END;
	} while (ret != Z_STREAM_END);

	inflateEnd(&stream);
	return true;
}

void UncompressFile(const char* input_file, const char* output_file) {
	FILE* bin_file = fopen(input_file, "rb");
	if (bin_file == nullptr) return;

	fseek(bin_file, 0, SEEK_END);
	size_t compressed_size = ftell(bin_file);
	rewind(bin_file);

	uint8_t* compressed_data = new uint8_t[compressed_size];
	fread(compressed_data, sizeof(uint8_t), compressed_size, bin_file);
	fclose(bin_file);

	vector<uint8_t> uncompressed_data;
	if (ZlibUncompress(compressed_data, compressed_size, uncompressed_data)) {
		FILE* tdbin_file = fopen(output_file, "wb");
		if (tdbin_file != nullptr) {
			fwrite(uncompressed_data.data(), sizeof(uint8_t), uncompressed_data.size(), tdbin_file);
			fclose(tdbin_file);
		}
	}
	delete[] compressed_data;
}

bool IsFileCompressed(const char* filename) {
	FILE* test_file = fopen(filename, "rb");
	if (test_file == nullptr) return false;
	uint8_t header[5];
	fread(header, sizeof(uint8_t), 5, test_file);
	fclose(test_file);
	return strncmp((char*)header, "TDBIN", sizeof(header)) != 0;
}
