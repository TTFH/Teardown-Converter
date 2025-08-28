#ifndef ZLIB_INFLATE_H
#define ZLIB_INFLATE_H

#include <vector>
#include <stdint.h>

using namespace std;

bool ZlibBlockCompress(const uint8_t* source, size_t source_len, int level, vector<uint8_t>& dest);
bool ZlibUncompress(const uint8_t* source, const size_t source_len, vector<uint8_t>& dest);
void UncompressFile(const char* input_file, const char* output_file);
bool IsFileCompressed(const char* filename);

#endif
