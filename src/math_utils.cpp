#include <math.h>

#include "math_utils.h"

double deg(double rad) {
	return rad * 180.0 / PI;
}

double rad(double deg) {
	return deg * (PI / 180.0);
}

bool CompareFloat(float a, float b) {
	return fabs(a - b) < 0.0001;
}

float Vec3::length() const {
	return sqrt(x * x + y * y + z * z);
}

bool Vec3::isZero() const {
	return *this == Vec3();
}

Vec3 Vec3::normalize() const {
	float len = length();
	if (len == 0) return Vec3();
	return Vec3(x / len, y / len, z / len);
}

bool Vec3::operator==(const Vec3& v) const {
	return CompareFloat(x, v.x) && CompareFloat(y, v.y) && CompareFloat(z, v.z);
}

bool Vec3::operator!=(const Vec3& v) const {
	return !(*this == v);
}

Vec3 Vec3::operator+(const Vec3& v) const {
	return Vec3(x + v.x, y + v.y, z + v.z);
}

Vec3 Vec3::operator-(const Vec3& v) const {
	return Vec3(x - v.x, y - v.y, z - v.z);
}

Vec3 Vec3::operator*(float f) const {
	return Vec3(x * f, y * f, z * f);
}

static Quat Conjugate(const Quat& q) {
	return {-q.x, -q.y, -q.z, q.w};
}

Quat operator*(const Quat& p, const Quat& q) {
	double x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
	double y = p.w * q.y - p.x * q.z + p.y * q.w + p.z * q.x;
	double z = p.w * q.z + p.x * q.y - p.y * q.x + p.z * q.w;
	double w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
	return Quat(x, y, z, w);
}

Vec3 operator*(const Quat& q, const Vec3& p1) {
	Quat p(p1.x, p1.y, p1.z, 0);
	Quat qConj = Conjugate(q);
	Quat result = q * p * qConj;
	return Vec3(result.x, result.y, result.z);
}

Quat QuatEuler(double roll, double yaw, double pitch) {
	return QuatEulerRad(rad(roll), rad(yaw), rad(pitch));
}

Quat QuatEulerRad(double roll, double yaw, double pitch) {
	double c1 = cos(roll / 2.0);
	double s1 = sin(roll / 2.0);
	double c2 = cos(yaw / 2.0);
	double s2 = sin(yaw / 2.0);
	double c3 = cos(pitch / 2.0);
	double s3 = sin(pitch / 2.0);

	return Quat(
		s1 * c2 * c3 + c1 * s2 * s3,
		c1 * s2 * c3 + s1 * c2 * s3,
		c1 * c2 * s3 - s1 * s2 * c3,
		c1 * c2 * c3 - s1 * s2 * s3
	);
}

// Safe range:
// x, y = [0, 360], z = [-90, 90]
// x, y = (-180, 180), z = (-90, 90)
Vec3 QuatToEuler(Quat q) {
	float bank, heading, attitude;
	double x = q.x, y = q.y, z = q.z, w = q.w;
	double s = 2 * x * y + 2 * z * w;
	if (s >= 0.999) {
		bank = 0;
		heading = 2 * atan2(x, w);
		attitude = PI / 2;
	} else if (s <= -0.999) {
		bank = 0;
		heading = -2 * atan2(x, w);
		attitude = -PI / 2;
	} else {
		bank = atan2(2 * x * w - 2 * y * z, 1 - 2 * x * x - 2 * z * z);
		heading = atan2(2 * y * w - 2 * x * z, 1 - 2 * y * y - 2 * z * z);
		attitude = asin(s);
	}
	bank = deg(bank);
	heading = deg(heading);
	attitude = deg(attitude);
	return Vec3(bank, heading, attitude);
}

Quat FromAxisAngle(Vec3 axis, float angle) {
	axis = axis.normalize();
	angle = rad(angle);
	float s = sin(angle / 2.0);
	float c = cos(angle / 2.0);
	return Quat(axis.x * s, axis.y * s, axis.z * s, c);
}

// Transform one transform into the local space of another transform.
Transform TransformToLocalTransform(const Transform& parent, const Transform& child) {
	Transform local_tr;
	Quat inv_parent_rot = Conjugate(parent.rot);
	local_tr.pos = inv_parent_rot * (child.pos - parent.pos);
	local_tr.rot = inv_parent_rot * child.rot;
	return local_tr;
}

bool Transform::isDefault() {
	return pos.isZero() && CompareFloat(rot.x, 0) && CompareFloat(rot.y, 0) && CompareFloat(rot.z, 0) && CompareFloat(rot.w, 1);
}

Tensor3D::Tensor3D() : sizex(0), sizey(0), sizez(0) {}

Tensor3D::Tensor3D(int sizex, int sizey, int sizez) : sizex(sizex), sizey(sizey), sizez(sizez) {
	data.resize(sizex * sizey * sizez, 0);
}

void Tensor3D::FromRunLengthEncoding(const RLE& rle) {
	int k = 0;
	for (RLE::const_iterator it = rle.begin(); it != rle.end(); it++) {
		uint8_t run_length = it->first;
		uint8_t entry = it->second;
		for (int j = 0; j <= run_length; j++)
			data[k++] = entry;
	}
}

void Tensor3D::Set(int x, int y, int z, uint8_t value) {
	data[x + sizex * (y + sizey * z)] = value;
}

uint8_t Tensor3D::Get(int x, int y, int z) const {
	return data[x + sizex * (y + sizey * z)];
}

bool Tensor3D::IsFilledSingleColor() const {
	uint8_t color = data[0];
	for (size_t i = 1; i < data.size(); i++)
		if (data[i] != color)
			return false;
	return true;
}

int Tensor3D::GetVolume() const {
	return data.size();
}

int Tensor3D::GetNonZeroCount() const {
	int count = 0;
	for (size_t i = 1; i < data.size(); i++)
		if (data[i] != 0)
			count++;
	return count;
}

const uint8_t* Tensor3D::ToArray() const {
	return data.data();
}
