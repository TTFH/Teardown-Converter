#ifndef TEARDOWN_STRUCTS_H
#define TEARDOWN_STRUCTS_H

#include <stdint.h>

enum GameState {
	Splash = 1,
	Menu = 2,
	UI = 3, // ??
	Playing = 4,
	Editing = 5,
	Quitting = 6
};

enum MaterialKind {
	None,
	Glass,
	Wood,
	Masonry,
	Plaster,
	Metal,
	HeavyMetal,
	Rock,
	Dirt,
	Foliage,
	Plastic,
	HardMetal,
	HardMasonry,
	Ice,
	Unphysical
};

struct Vector {
	float x, y, z;
};

struct Quat {
	float x, y, z, w;
};

struct Transform {
	Vector pos;
	Quat rot;
};

struct RGBA {
	float r, g, b, a;
};

struct Material {
	uint32_t kind;
	RGBA rgba;
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;
	uint32_t replaceable;
};

struct Palette {
	uint32_t padding1[3];
	Material materials[256];
	uint8_t tint_table[2 * 4 * 256];
	uint32_t padding2;
};

struct Entity {
	void* unknown;		// 0x00
	uint8_t kind;		// 0x08
	uint8_t flags;		// 0x09
	uint8_t padding[2];
	uint32_t handle;	// 0x0C
	Entity* parent;		// 0x10
	Entity* sibling;	// 0x18
	Entity* child;		// 0x20
}; // size: 0x20

struct Vox {
	uint32_t size[3];		// 0x00
	uint8_t padding1[4];
	uint8_t* voxels;		// 0x10
	void* physics_buffer;
	float scale;			// 0x20
	uint8_t unknown1[48];	// ????
	uint32_t palette;		// 0x54
	uint8_t unknown2[4];	// ????
	int32_t voxel_count;	// 0x5C
}; // size: 0x60

struct Shape {
	Entity self;
	Transform local_tr;			// 0x28
	Vector min_aabb;			// 0x44
	Vector max_aabb;			// 0x50
	uint8_t padding1[4];
	void* dormant;				// 0x60
	void* active;				// 0x68
	uint16_t collision_flags;	// 0x70
	uint8_t collision_layer;	// 0x72
	uint8_t collision_mask;		// 0x73
	float density;				// 0x74
	float strength;				// 0x78
	uint16_t texture_tile;		// 0x7C
	uint16_t blendtexture_tile;	// 0x7E
	float texture_weight;		// 0x80
	float blendtexture_weight;	// 0x84
	Vector starting_wpos;		// 0x88
	uint8_t padding2[4];
	Vox* vox;					// 0x98
	void* joints;				// 0xA0
	float emissive_scale;		// 0xA8
	bool broken;				// 0xAC
	uint8_t padding3[3];
	Transform world_tr;			// 0xB0
	uint8_t padding4[4];
}; // size: 0xD0

class td_string {
	union {
		char* HeapBuffer;
		char StackBuffer[16] = { 0 };
	};
public:
	const char* c_str() const {
		return StackBuffer[15] != '\0' ? HeapBuffer : &StackBuffer[0];
	}
};

template<typename T>
class td_vector {
	uint32_t size = 0;
	uint32_t capacity = 0;
	T* data = nullptr;
public:
	uint32_t getSize() const {
		return size;
	}
	T operator[](uint32_t index) const {
		return data[index];
	}
	T& operator[](uint32_t index) {
		return data[index];
	}
};

struct LuaStateInfo {
	lua_State* state;
};

struct Script {
	Entity self;
	td_string name;				// 0x28
	uint8_t padding1[8];
	float runtime;				// 0x40
	float delta;
	td_string path;				// 0x48
	td_string location;			// 0x58
	uint8_t padding2[56];
	LuaStateInfo* state_info;	// 0xA0
	bool is_loaded;				// 0xA8
	bool has_init;
	bool has_tick;
	bool has_update;
	bool has_draw;
	bool has_command;
	uint8_t padding3[2];
	float runtime2;				// 0xB0
	float update_runtime;		// 0xB4
}; // size: ?

struct Game {
	uint32_t screen_width;		// 0x00
	uint32_t screen_height;		// 0x04
	uint32_t state;				// 0x08
	uint8_t padding1[64];
	void* scene;				// 0x48
};

RGBA operator*(const RGBA& color, float scale) {
	return { color.r * scale, color.g * scale, color.b * scale, color.a * scale };
}
RGBA operator+(const RGBA& color1, const RGBA& color2) {
	return { color1.r + color2.r, color1.g + color2.g, color1.b + color2.b, color1.a + color2.a };
}
RGBA operator-(const RGBA& color1, const RGBA& color2) {
	return { color1.r - color2.r, color1.g - color2.g, color1.b - color2.b, color1.a - color2.a };
}

#endif
