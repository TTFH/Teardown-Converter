#include <assert.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "binary_reader.h"

FileReader::FileReader() {
	file = nullptr;
}

void FileReader::InitReader(const char* path) {
	file = fopen(path, "rb");
	if (file == nullptr) {
	#ifdef _WIN32
		MessageBoxA(nullptr, "Could not open map file, check the game path", "File not found", MB_OK | MB_ICONERROR);
	#endif
		printf("[ERROR] Could not open %s for reading\n", path);
		exit(EXIT_FAILURE);
	}
}

FileReader::~FileReader() {
	fclose(file);
}

uint8_t FileReader::ReadByte() {
	uint8_t b;
	fread(&b, 1, 1, file);
	return b;
}

uint16_t FileReader::ReadWord() {
	uint16_t w;
	fread(&w, 2, 1, file);
	return w;
}

uint32_t FileReader::ReadInt() {
	uint32_t i;
	fread(&i, 4, 1, file);
	return i;
}

float FileReader::ReadFloat() {
	float f;
	fread(&f, 4, 1, file);
	return f;
}

double FileReader::ReadDouble() {
	double d;
	fread(&d, 8, 1, file);
	return d;
}

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

Vertex Reader::ReadVertex() {
	Vertex vertex;
	vertex.x = ReadFloat();
	vertex.y = ReadFloat();
	return vertex;
}

Vec2 Reader::ReadVec2() {
	Vec2 vec;
	vec.x = ReadFloat();
	vec.y = ReadFloat();
	return vec;
}

Vec3 Reader::ReadVec3() {
	Vec3 vec;
	vec.x = ReadFloat();
	vec.y = ReadFloat();
	vec.z = ReadFloat();
	return vec;
}

Vec4 Reader::ReadVec4() {
	Vec4 vec;
	vec.x = ReadFloat();
	vec.y = ReadFloat();
	vec.z = ReadFloat();
	vec.w = ReadFloat();
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
	transform.pos = ReadVec3();
	transform.rot = ReadQuat();
	return transform;
}

void Reader::ReadBuffer(uint8_t* buffer, int size) {
	for (int i = 0; i < size; i++)
		buffer[i] = ReadByte();
}
