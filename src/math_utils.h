#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#define PI 3.14159265

struct Vector {
	float x, y, z;
	Vector() : x(0), y(0), z(0) {}
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}
	Vector(float v[3]) : x(v[0]), y(v[1]), z(v[2]) {}
	float length();
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

double deg(double rad);
double rad(double deg);
Quat QuatEuler(double roll, double yaw, double pitch);
Quat QuatEulerRad(double roll, double yaw, double pitch);
void QuatToEuler(Quat q, float &bank, float &heading, float &attitude);
Transform TransformToLocalTransform(const Transform& parent, const Transform& child);

Vector operator+(const Vector& u, const Vector& v);
Vector operator-(const Vector& u, const Vector& v);
Quat operator*(const Quat& p, const Quat& q);
Vector operator*(const Quat& q, const Vector& p1);

#endif
