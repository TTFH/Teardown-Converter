#ifndef BINARY_READER_H
#define BINARY_READER_H

#include <string>
#include <stdio.h>
#include <stdint.h>

#include "scene.h"

using namespace std;

class Reader {
public:
	virtual uint8_t ReadByte() = 0;
	virtual uint16_t ReadWord() = 0;
	virtual uint32_t ReadInt() = 0;
	virtual float ReadFloat() = 0;
	virtual double ReadDouble() = 0;

	bool ReadBool();
	string ReadString();

	Tag ReadTag();
	Registry ReadRegistry();
	Color ReadColor();
	Vertex ReadVertex();
	Vec2 ReadVec2();
	Vec3 ReadVec3();
	Vec4 ReadVec4();
	Quat ReadQuat();
	Transform ReadTransform();
	void ReadBuffer(uint8_t* buffer, int size);

	virtual ~Reader() = default;
};

class FileReader : public Reader {
private:
	FILE* file;
public:
	FileReader();
	void InitReader(const char* path);
	uint8_t ReadByte() override;
	uint16_t ReadWord() override;
	uint32_t ReadInt() override;
	float ReadFloat() override;
	double ReadDouble() override;
	~FileReader();
};
/*
class BufferReader : public Reader {
private:
	uint8_t* buffer;
	int size;
	int offset;
public:
	BufferReader(const uint8_t* buffer, int size);
	uint8_t ReadByte() override;
	uint16_t ReadWord() override;
	uint32_t ReadInt() override;
	float ReadFloat() override;
	double ReadDouble() override;
	~BufferReader();
};
*/

#endif
