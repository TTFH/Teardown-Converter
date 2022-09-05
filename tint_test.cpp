#include <math.h>
#include <stdio.h>
#include <stdint.h>

enum Color { BLACK = 0, YELLOW };

class RGB {
private:
	float r, g, b;
public:
	RGB() : r(0), g(0), b(0) {}
	RGB(float r, float g, float b) : r(r), g(g), b(b) {}

	uint8_t red() const { return r * 255.0; }
	uint8_t green() const { return g * 255.0; }
	uint8_t blue() const { return b * 255.0; }

	void setRed(uint8_t r) { this->r = r / 255.0; }
	void setGreen(uint8_t g) { this->g = g / 255.0; }
	void setBlue(uint8_t b) { this->b = b / 255.0; }

	bool operator==(const RGB& other) const {
		//return red() == other.red() && green() == other.green() && blue() == other.blue();
		return fabs(red() - other.red()) < 2 && fabs(green() - other.green()) < 2 && fabs(blue() - other.blue()) < 2;
	}
	bool operator!=(const RGB& other) const {
		return !(*this == other);
	}
	RGB operator*(float f) const {
		return {r * f, g * f, b * f};
	}
	RGB operator+(const RGB& other) const {
		return {r + other.r, g + other.g, b + other.b};
	}
	RGB operator-(const RGB& other) const {
		return {r - other.r, g - other.g, b - other.b};
	}
};

const RGB YELLOW_TINT = {0.9, 0.7, 0.1};
const RGB BLACK_TINT = {0, 0, 0};

class GenerateTints {
private:
	RGB original_color;
	RGB tint_table[2 * 4];
public:
	GenerateTints(uint8_t serialized[27]) {
		original_color.setRed(serialized[0]);
		original_color.setGreen(serialized[1]);
		original_color.setBlue(serialized[2]);
		for (int i = 0; i < 2 * 4; i++) {
			tint_table[i].setRed(serialized[3 + i * 3]);
			tint_table[i].setGreen(serialized[4 + i * 3]);
			tint_table[i].setBlue(serialized[5 + i * 3]);
		}
	}

	RGB paint(Color color, uint8_t strength) { // strength in range [0-4]
		if (color == BLACK) {
			RGB diff = (BLACK_TINT - original_color) * 0.1;
			return original_color + diff * strength;
		} else {
			RGB diff = (YELLOW_TINT - original_color) * 0.25;
			//printf("diff: %d %d %d\n", diff.red(), diff.green(), diff.blue());
			if (diff.red() < 10 && diff.green() < 10 && diff.blue() < 10)
				strength = 2;
			return original_color + diff * strength;
		}
	}

	void test() {
		int sucess = 0;
		for (int i = 0; i < 2 * 4; i++) {
			RGB tint = paint((Color)(i / 4), i % 4 + 1);
			if (i / 4 == 0) continue; // skip black in testing
			if (tint_table[i] != tint) {
				printf("%s %d/4 Test failed.\n", i / 4 == 0 ? "BLACK" : "YELLOW", i % 4 + 1);
				printf("\tExpected: %d %d %d\n", tint_table[i].red(), tint_table[i].green(), tint_table[i].blue());
				printf("\tGot: %d %d %d\n\n", tint.red(), tint.green(), tint.blue());
			} else
				sucess++;
		}
		printf("Successfull Tests: %d/4\n\n", sucess);
	}
};

void executeTest(uint8_t serialized[27]) {
	GenerateTints tints(serialized);
	tints.test();
}

int main() {
	uint8_t test1[27] = {
		255, 255, 255,
		// BLACK
		229, 229, 229,
		203, 203, 203,
		177, 177, 177,
		151, 151, 151,
		// YELLOW
		248, 235, 197,
		242, 216, 140,
		235, 197, 82,
		229, 178, 25,
	};

	uint8_t test2[27] = {
		204, 178, 0,
		// BLACK
		183, 161, 2,
		163, 144, 5,
		143, 126, 8,
		123, 109, 11,
		// YELLOW
		216, 178, 12,
		216, 178, 12,
		216, 178, 12,
		216, 178, 12,
	};

	uint8_t test3[27] = {
		102, 127, 153,
		// BLACK
		93, 116, 138,
		84, 104, 124,
		76, 93, 109,
		67, 81, 95,
		// YELLOW
		133, 140, 121,
		165, 153, 89,
		197, 165, 57,
		229, 178, 25,
	};

	executeTest(test1);
	executeTest(test2);
	executeTest(test3);
	return 0;
}
