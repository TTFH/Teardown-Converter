#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>
#include <vector>
#include <utility>

using namespace std;

#define PI 3.14159265

typedef vector<pair<uint8_t, uint8_t>> RLE;

struct Vector {
	float x, y, z;
	Vector() : x(0), y(0), z(0) {}
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}
	Vector(float v[3]) : x(v[0]), y(v[1]), z(v[2]) {}
	float length();
	bool nonZero();
	bool operator==(const Vector& v);
	Vector operator+(const Vector& v);
	Vector operator-(const Vector& v) const;
	Vector operator*(float f);
};

struct Quat {
	float x, y, z, w;
	Quat() : x(0), y(0), z(0), w(1) {}
	Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Transform {
	Vector pos;
	Quat rot;
	Transform() : pos(), rot() {}
	Transform(Vector pos, Quat rot) : pos(pos), rot(rot) {}
};

class Tensor3D {
public:
	int sizex, sizey, sizez;
	uint8_t*** data = NULL;

	Tensor3D(int sizex, int sizey, int sizez);
	~Tensor3D();

	void FromRunLengthEncoding(RLE rle);
	void Set(int x, int y, int z, uint8_t value);
	uint8_t Get(int x, int y, int z) const;
	bool isFilledSingleColor();
	int GetVolume();
	int GetNonZeroCount();
	uint8_t* ToArray();
};

double deg(double rad);
double rad(double deg);
Quat QuatEuler(double roll, double yaw, double pitch);
Quat QuatEulerRad(double roll, double yaw, double pitch);
void QuatToEuler(Quat q, float &bank, float &heading, float &attitude);
Transform TransformToLocalTransform(const Transform& parent, const Transform& child);

Quat operator*(const Quat& p, const Quat& q);
Vector operator*(const Quat& q, const Vector& p1);

#endif
