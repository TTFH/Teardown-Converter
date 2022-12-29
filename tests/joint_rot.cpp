#include <math.h>
#include <stdio.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

string FloatToString(float value) {
	if (fabs(value) < 0.001) value = 0;
	stringstream ss;
	ss << fixed << setprecision(3) << value;
	string str = ss.str();
	if (str.find('.') != string::npos) {
		str = str.substr(0, str.find_last_not_of('0') + 1);
		if (str.find('.') == str.size() - 1)
			str = str.substr(0, str.size() - 1);
	}
	return str;
}

#define PI 3.14159265

float deg(float rad) {
	return rad * 180.0 / PI;
}

float rad(float deg) {
	return deg * (PI / 180.0);
}

struct Vector {
	float x, y, z;
	Vector() : x(0), y(0), z(0) {}
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}
};

// Rotation in radians
Vector GetJointAxis(float x, float y, float z) {
	float a = sin(x) * cos(y) * sin(z) + cos(x) * sin(y);
	float b = -sin(x) * cos(z);
	float c = -sin(x) * sin(y) * sin(z) + cos(x) * cos(y);
	return Vector(a, b, c);
}

void TestJointAxis(float x, float y, float z) {
	Vector v = GetJointAxis(rad(x), rad(y), rad(z));
	printf("%4g %4g %4g\t%4s %4s %4s\n", x, y, z, FloatToString(v.x).c_str(), FloatToString(v.y).c_str(), FloatToString(v.z).c_str());
}

int main() {
	TestJointAxis(0, 0, 0);
	printf("\n");
	TestJointAxis(0, 0, 90);
	TestJointAxis(0, 0, -90);
	TestJointAxis(0, 0, 180);
	printf("\n");
	TestJointAxis(0, 90, 0);
	TestJointAxis(0, -90, 0);
	TestJointAxis(0, 180, 0);
	printf("\n");
	TestJointAxis(90, 0, 0);
	TestJointAxis(-90, 0, 0);
	TestJointAxis(180, 0, 0);
	printf("\n");
	TestJointAxis(0, 90, 90);
	TestJointAxis(90, 0, 90);
	TestJointAxis(90, 90, 0);
	TestJointAxis(90, 90, 90);
	printf("\n");
	TestJointAxis(0, -90, -90);
	TestJointAxis(-90, 0, -90);
	TestJointAxis(-90, -90, 0);
	TestJointAxis(-90, -90, -90);

	return 0;
}

/*
	a	b	c		x	y	z
	0	0	0		0	0	1

	0	0  90		0	0	1
	0	0 -90		0	0	1
	0	0 180		0	0	1

	0  90	0		1	0	0
	0 -90	0	   -1	0	0
	0 180	0		0	0  -1

   90	0	0		0  -1	0
  -90	0	0		0	1	0
  180	0	0		0	0  -1

	0  90  90		1	0	0
   90	0  90		1	0	0
   90  90	0		0  -1	0
   90  90  90		0	0  -1

	0 -90 -90	   -1	0	0
  -90	0 -90		1	0	0
  -90 -90	0		0	1	0
  -90 -90 -90		0	0	1


	a	b	c		x	y	z
	0  90	0		1	0	0
	0 -90	0	   -1	0	0
  -90	0	0		0	1	0
   90	0	0		0  -1	0
	0	0	0		0	0	1
	0 180	0		0	0  -1
*/
