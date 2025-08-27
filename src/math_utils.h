#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>
#include <vector>

using namespace std;

#define PI 3.14159265358979323846

typedef vector<pair<uint8_t, uint8_t>> RLE;

struct Vec3 {
	float x, y, z;
	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3(float v[3]) : x(v[0]), y(v[1]), z(v[2]) {}
	float length() const;
	bool isZero() const;
	Vec3 normalize() const;
	bool operator==(const Vec3& v) const;
	bool operator!=(const Vec3& v) const;
	Vec3 operator+(const Vec3& v) const;
	Vec3 operator-(const Vec3& v) const;
	Vec3 operator*(float f) const;
};

struct Quat {
	float x, y, z, w;
	Quat() : x(0), y(0), z(0), w(1) {}
	Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Transform {
	Vec3 pos;
	Quat rot;
	Transform() : pos(), rot() {}
	Transform(Vec3 pos, Quat rot) : pos(pos), rot(rot) {}
	bool isDefault();
};

class Tensor3D {
private:
	uint8_t*** data = NULL;
public:
	int sizex, sizey, sizez;

	Tensor3D(int sizex, int sizey, int sizez);
	void Clear();

	void FromRunLengthEncoding(const RLE& rle);
	void Set(int x, int y, int z, uint8_t value);
	uint8_t Get(int x, int y, int z) const;
	bool IsFilledSingleColor() const;
	int GetVolume() const;
	int GetNonZeroCount() const;
	uint8_t* ToArray() const;
};

double deg(double rad);
double rad(double deg);
bool CompareFloat(float a, float b);
Quat QuatEuler(double roll, double yaw, double pitch);
Quat QuatEulerRad(double roll, double yaw, double pitch);
void QuatToEuler(Quat q, float &bank, float &heading, float &attitude);
Quat FromAxisAngle(Vec3 axis, float angle);
Transform TransformToLocalTransform(const Transform& parent, const Transform& child);

Quat operator*(const Quat& p, const Quat& q);
Vec3 operator*(const Quat& q, const Vec3& p1);

#endif
