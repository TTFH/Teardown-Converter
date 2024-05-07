#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>
#include <string>

#include "entity.h"

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

struct PostProcessing {
	float brightness = 1;				// brightness
	Color colorbalance = {1, 1, 1, 1};	// colorbalance
	float saturation = 1;				// saturation
	float gamma = 1;					// gamma
	float bloom = 1;					// bloom
};

struct Player {
	Transform transform;
	float yaw;
	float pitch;
	Vector velocity;
	float health;
	float transition_time;
	float bluetide_timer;
	float z_f32_1;
	float z_f32_2;
	/*
	HealthRegeneration: bool
	WalkingSpeed: float
	JumpSpeed: float
	GodMode: bool
	FlyMode: bool
	*/
};

struct Sun {
	float tint_brightness[3];
	Color colorTint;	// sunColorTint
	Vector dir;			// sunDir
	float brightness;	// sunBrightness
	float spread;		// sunSpread
	float length;		// sunLength
	float fogScale = 1;	// sunFogScale
	float glare;		// sunGlare
};

struct Skybox {
	string texture;			// skybox
	Color tint;				// skyboxtint
	float brightness = 1;	// skyboxbrightness
	float rot;				// skyboxrot in radians
	Sun sun;
	uint8_t z_u8;
	Color constant = {0.003, 0.003, 0.003, 1};	// constant
	float ambient;								// ambient
	float ambientexponent = 1.3;				// ambientexponent
};

struct Fog {
	Color color = {1, 1, 1, 1};	// fogColor
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
	Vector dir = {0, -1, 0};	// snowdir x y z
	float spread = 0.2;			// snowdir spread
	float amount = 0;			// snowamount
	float speed;
	bool onground = false;		// snowonground
};

struct Environment {
	Skybox skybox;
	float exposure[2];	// exposure
	float brightness;	// brightness
	Fog fog;
	EnvWater water;
	bool nightlight;	// nightlight
	Sound ambience;		// ambience
	float slippery = 0;	// slippery
	float fogscale = 1;	// fogscale
	Snow snow;
	Vector wind;		// wind
	float waterhurt = 0;// waterhurt
};

struct Boundary {
	Vec<Vertex> vertices;
	float padleft = -5;		// negative
	float padtop = -5;		// negative
	float padright = 5;
	float padbottom = 5;
	float maxheight = 0;
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
	uint8_t other_tint[4 * 256];
};

struct Scene {
	char magic[5];					// TDBIN
	uint8_t version[3] = {0, 3, 0};	// version
	string game_levelid;
	string game_levelpath;
	string layers;
	string game_mod;
	uint32_t z1_u32;
	Vec<Registry> enabled_mods;
	Vec<Registry> spawned_mods;
	uint32_t driven_vehicle = 0;	// driven
	Vector shadow_volume;			// shadowVolume
	Transform spawnpoint;			// spawnpoint
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
