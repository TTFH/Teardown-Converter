#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>
#include <iosfwd>
#include <string>

#include "entity.h"
#include "math_utils.h"

using namespace std;

enum class MaterialKind : uint8_t {
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

extern const char* MaterialKindName[];

bool operator==(uint8_t lhs, const MaterialKind& rhs);

struct Player {
	Transform transform;
	float yaw;
	float pitch;
	Vector velocity;
	float health;
	float z_f32_4[4];
};

struct Sun {
	float tint_brightness[3];
	Color colorTint;	// sunColorTint
	Vector dir;			// sunDir
	float brightness;	// sunBrightness
	float spread;		// sunSpread
	float length;		// sunLength
	float fogScale;		// sunFogScale
	float glare;		// sunGlare
};

struct Skybox {
	string texture;			// skybox
	Color tint;				// skyboxtint
	float brightness;		// skyboxbrightness
	float rot;				// skyboxrot in radians
	Sun sun;
	uint8_t z_u8;
	Color constant;			// constant
	float ambient;			// ambient
	float ambientexponent;	// ambientexponent
};

struct Fog {
	Color color;			// fogColor
	// fogParams:
	float start;
	float distance;		// end = start + distance
	float amount;
	float exponent;
};

struct EnvWater {
	float wetness;		// wetness
	float puddleamount;	// puddleamount
	float puddlesize;	// puddlesize =  0.01 / this
	float rain;			// rain
};

struct Snow {
	Vector dir;		// snowdir x y z
	float spread;	// snowdir spread
	float amount;	// snowamount
	float speed;
	bool onground;	// snowonground
};

struct Environment {
	Skybox skybox;
	float exposure[2];	// exposure
	float brightness;	// brightness
	Fog fog;
	EnvWater water;
	bool nightlight;	// nightlight
	Sound ambience;		// ambience
	float slippery;		// slippery
	float fogscale;		// fogscale
	Snow snow;
	Vector wind;		// wind
	float waterhurt;	// waterhurt
};

struct Boundary {
	Vec<Vertex> vertices;
	float padleft;		// negative
	float padtop;		// negative
	float padright;
	float padbottom;
};

struct Fire {
	uint32_t entity_handle;
	Vector pos;
	float max_time;
	float time;
	uint8_t z_u8_6[6];
};

struct Material {
	uint8_t kind; // MaterialKind
	Color rgba;
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;	// [0..32]
	bool replaceable;
};

struct Palette {
	Material materials[256];
	uint8_t z_u8;
	uint8_t black_tint[4 * 256];
	uint8_t yellow_tint[4 * 256];
};

struct PostProcessing {
	float brightness;		// brightness
	Color colorbalance;		// colorbalance
	float saturation;		// saturation
	float gamma;			// gamma
	float bloom;			// bloom
};

struct Scene {
	char magic[5];			// TDBIN
	uint8_t version[3];		// version
	string level;
	uint32_t driven_vehicle;// driven
	Vector shadow_volume;	// shadowVolume
	Transform spawnpoint;	// spawnpoint
	// Pattern: 1  n  n-5  n-1
	uint32_t world_body_handle;
	uint32_t flashlight_handle;
	uint32_t explosion_lua_handle;
	uint32_t achievements_lua_handle;

	PostProcessing postpro;
	Player player;
	Environment environment;
	Boundary boundary;
	Vec<Fire> fires;
	Vec<Palette> palettes;
	Vec<Registry> registry;
	Vec<Entity*> entities;
	uint32_t entity_count;
	uint8_t padding[9];
};

#endif
