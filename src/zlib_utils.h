#ifndef ZLIB_INFLATE_H
#define ZLIB_INFLATE_H

#include <stdint.h>

bool ZlibBlockCompress(const uint8_t* source, int source_len, int level, uint8_t* dest, int &dest_len);
bool ZlibUncompress(const uint8_t* source, long int source_len, uint8_t* dest, long int &dest_len);
void UncompressFile(const char* input_file, const char* output_file);
bool IsFileCompressed(const char* filename);

#endif
