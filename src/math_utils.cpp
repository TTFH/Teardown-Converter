#include <math.h>

#include "math_utils.h"

using namespace std;

double deg(double rad) {
	return rad * 180.0 / PI;
}

double rad(double deg) {
	return deg * (PI / 180.0);
}

float Vector::length() {
	return sqrt(x * x + y * y + z * z);
}

bool Vector::nonZero() {
	return x != 0 || y != 0 || z != 0;
}

static bool CompareFloat(float a, float b) {
	return fabs(a - b) < 0.0001;
}

bool Vector::operator==(const Vector& v) {
	return CompareFloat(x, v.x) && CompareFloat(y, v.y) && CompareFloat(z, v.z);
}

Vector Vector::operator+(const Vector& v) {
	return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector::operator-(const Vector& v) const {
	return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector::operator*(float f) {
	return Vector(x * f, y * f, z * f);
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

Vector operator*(const Quat& q, const Vector& p1) {
	Quat p(p1.x, p1.y, p1.z, 0);
	Quat qConj = Conjugate(q);
	Quat result = q * p * qConj;
	return Vector(result.x, result.y, result.z);
}

Quat QuatEuler(double roll, double yaw, double pitch) {
	double c1 = cos(rad(roll) / 2.0);
	double s1 = sin(rad(roll) / 2.0);
	double c2 = cos(rad(yaw) / 2.0);
	double s2 = sin(rad(yaw) / 2.0);
	double c3 = cos(rad(pitch) / 2.0);
	double s3 = sin(rad(pitch) / 2.0);

	return Quat(
		s1 * c2 * c3 + c1 * s2 * s3,
		c1 * s2 * c3 + s1 * c2 * s3,
		c1 * c2 * s3 - s1 * s2 * c3,
		c1 * c2 * c3 - s1 * s2 * s3
	);
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

Tensor3D::Tensor3D(int sizex, int sizey, int sizez) {
	this->sizex = sizex;
	this->sizey = sizey;
	this->sizez = sizez;

	data = new uint8_t**[sizex];
	for (int i = 0; i < sizex; i++) {
		data[i] = new uint8_t*[sizey];
		for (int j = 0; j < sizey; j++)
			data[i][j] = new uint8_t[sizez];
	}
	for (int i = 0; i < sizex; i++)
		for (int j = 0; j < sizey; j++)
			for (int k = 0; k < sizez; k++)
				data[i][j][k] = 0x00;
}

Tensor3D::~Tensor3D() {
	/*for (int i = 0; i < sizex; i++) {
		for (int j = 0; j < sizey; j++)
			delete[] data[i][j];
		delete[] data[i];
	}
	delete[] data;
	data = NULL;*/
}

void Tensor3D::FromRunLengthEncoding(RLE rle) {
	uint8_t* array = new uint8_t[GetVolume()];

	int k = 0;
	for (RLE::iterator it = rle.begin(); it != rle.end(); it++) {
		uint8_t run_length = it->first;
		uint8_t entry = it->second;
		for (unsigned int j = 0; j <= run_length; j++)
			array[k++] = entry;
	}

	k = 0;
	for (int z = 0; z < sizez; z++)
		for (int y = 0; y < sizey; y++)
			for (int x = 0; x < sizex; x++)
				data[x][y][z] = array[k++];

	delete[] array;
}

void Tensor3D::Set(int x, int y, int z, uint8_t value) {
	data[x][y][z] = value;
}

uint8_t Tensor3D::Get(int x, int y, int z) const {
	return data[x][y][z];
}

bool Tensor3D::isFilledSingleColor() {
	uint8_t color = data[0][0][0];
	for (int x = 0; x < sizex; x++)
		for (int y = 0; y < sizey; y++)
			for (int z = 0; z < sizez; z++)
				if (data[x][y][z] != color)
					return false;
	return true;
}

int Tensor3D::GetVolume() {
	return sizex * sizey * sizez;
}

int Tensor3D::GetNonZeroCount() {
	int count = 0;
	for (int x = 0; x < sizex; x++)
		for (int y = 0; y < sizey; y++)
			for (int z = 0; z < sizez; z++)
				if (data[x][y][z] != 0)
					count++;
	return count;
}

uint8_t* Tensor3D::ToArray() {
	int volume = GetVolume();
	if (volume == 0) return NULL;
	uint8_t* array = new uint8_t[volume];
	int i = 0;
	for (int z = 0; z < sizez; z++)
		for (int y = 0; y < sizey; y++)
			for (int x = 0; x < sizex; x++)
				array[i++] = data[x][y][z];
	return array;
}
