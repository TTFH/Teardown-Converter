#ifndef ZLIB_INFLATE_H
#define ZLIB_INFLATE_H

#include <stdint.h>

bool ZlibUncompress(const uint8_t* source, long int source_len, uint8_t* dest, long int &dest_len);
void UncompressFile(const char* input_file, const char* output_file);
bool IsFileCompressed(const char* filename);

#endif
