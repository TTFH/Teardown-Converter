#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdexcept>

#include "scene.h"

using namespace std;

// WIP, unused

class Reader {
public:
	virtual uint8_t ReadByte() = 0;
	virtual uint16_t ReadWord() = 0;
	virtual uint32_t ReadInt() = 0;
	virtual float ReadFloat() = 0;
	virtual double ReadDouble() = 0;

	// TODO: move / remove
	bool ReadBool();
	string ReadString();
	Tag ReadTag();
	Registry ReadRegistry();
	Color ReadColor();
	Vector ReadVector();
	Quat ReadQuat();
	Transform ReadTransform();
	void ReadBuffer(uint8_t* buffer, int size);

	virtual ~Reader() = default;
};

class FileReader : public Reader {
private:
	FILE* file;
public:
	FileReader(const char* path);
	uint8_t ReadByte() override;
	uint16_t ReadWord() override;
	uint32_t ReadInt() override;
	float ReadFloat() override;
	double ReadDouble() override;
	~FileReader();
};

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

// Too much overhead???

bool Reader::ReadBool() {
	uint8_t b = ReadByte();
	assert(b == 0 || b == 1);
	return b != 0;
}

string Reader::ReadString() {
	string str;
	char c;
	do {
		c = ReadByte();
		if (c != '\0') str += c;
	} while (c != '\0');
	return str;
}

Tag Reader::ReadTag() {
	Tag tag;
	tag.name = ReadString();
	tag.value = ReadString();
	return tag;
}

Registry Reader::ReadRegistry() {
	Registry entry;
	entry.key = ReadString();
	entry.value = ReadString();
	return entry;
}

Color Reader::ReadColor() {
	Color color;
	color.r = ReadFloat();
	color.g = ReadFloat();
	color.b = ReadFloat();
	color.a = ReadFloat();
	return color;
}

Vector Reader::ReadVector() {
	Vector vec;
	vec.x = ReadFloat();
	vec.y = ReadFloat();
	vec.z = ReadFloat();
	return vec;
}

Quat Reader::ReadQuat() {
	Quat quat;
	quat.x = ReadFloat();
	quat.y = ReadFloat();
	quat.z = ReadFloat();
	quat.w = ReadFloat();
	return quat;
}

Transform Reader::ReadTransform() {
	Transform transform;
	transform.pos = ReadVector();
	transform.rot = ReadQuat();
	return transform;
}

void Reader::ReadBuffer(uint8_t* buffer, int size) {
	for (int i = 0; i < size; i++)
		buffer[i] = ReadByte();
}
