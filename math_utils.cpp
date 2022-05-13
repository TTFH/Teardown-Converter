#include <math.h>

#include "math_utils.h"

double deg(double rad) {
	return rad * 180.0 / PI;
}

double rad(double deg) {
	return deg * (PI / 180.0);
}

Vector operator+(const Vector& u, const Vector& v) {
	return {u.x + v.x, u.y + v.y, u.z + v.z};
}

Vector operator-(const Vector& u, const Vector& v) {
	return {u.x - v.x, u.y - v.y, u.z - v.z};
}

Quat Conjugate(const Quat& q) {
	return {-q.x, -q.y, -q.z, q.w};
}

Quat operator*(const Quat& p, const Quat& q) {
	double x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
	double y = p.w * q.y - p.x * q.z + p.y * q.w + p.z * q.x;
	double z = p.w * q.z + p.x * q.y - p.y * q.x + p.z * q.w;
	double w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
	return Quat(x, y, z, w);
}

Vector operator*(const Quat& q, const Vector& p1) {
	Quat p = {p1.x, p1.y, p1.z, 0};
	Quat qConj = Conjugate(q);
	Quat result = q * p * qConj;
	return Vector(result.x, result.y, result.z);
}

Quat QuatEuler(double roll, double yaw, double pitch) {
	double c1 = cos(rad(roll) / 2);
	double s1 = sin(rad(roll) / 2);
	double c2 = cos(rad(yaw) / 2);
	double s2 = sin(rad(yaw) / 2);
	double c3 = cos(rad(pitch) / 2);
	double s3 = sin(rad(pitch) / 2);

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
void QuatToEuler(Quat q, float &bank, float &heading, float &attitude) {
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
}

// Transform one transform into the local space of another transform.
Transform TransformToLocalTransform(const Transform& parent, const Transform& child) {
	Transform local_tr;
	Quat inv_parent_rot = Conjugate(parent.rot);
	local_tr.pos = inv_parent_rot * (child.pos - parent.pos);
	local_tr.rot = inv_parent_rot * child.rot;
	return local_tr;
}
