#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>
#include <string>

#include "entity.h"

using namespace std;

enum MaterialKind : uint8_t {
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
	float brightness;	// brightness
	Color colorbalance;	// colorbalance
	float saturation;	// saturation
	float gamma;		// gamma
	float bloom;		// bloom
};

struct Player {
	Transform transform;
	float pitch;
	float yaw;
	Vector velocity;
	float health;
	float transition_timer;
	float time_underwater;
	float bluetide_timer;
	float bluetide_power;
};

struct Sun {
	Vector tint_brightness;
	Color colortint;	// sunColorTint
	Vector dir;			// sunDir
	float brightness;	// sunBrightness
	float spread;		// sunSpread
	float length;		// sunLength
	float fogscale;		// sunFogScale
	float glare;		// sunGlare
};

struct Skybox {
	string texture;			// skybox
	Color tint;				// skyboxtint
	float brightness;		// skyboxbrightness
	float rot;				// skyboxrot in radians
	Sun sun;
	bool auto_sun_dir;
	Color constant;			// constant
	float ambient;			// ambient
	float ambientexponent;	// ambientexponent
};

struct Fog {
	Color color;	// fogColor
	// fogParams:
	float start;
	float distance;	// end = start + distance
	float amount;
	float exponent;
};

namespace Env {
	struct Water {
		float wetness;		// wetness
		float puddleamount;	// puddleamount
		float puddlesize;	// puddlesize = 0.01 / this
		float rain;			// rain
	};
}

struct Snow {
	Vector dir;		// snowdir x y z
	float spread;	// snowdir spread
	float amount;	// snowamount
	float speed;	// snowamount
	bool onground;	// snowonground
};

struct Environment {
	Skybox skybox;
	float exposure[2];	// exposure
	float brightness;	// brightness
	Fog fog;
	Env::Water water;
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
	float maxheight;
};

struct Fire {
	uint32_t shape;
	Vector position;
	float max_time;
	float time;
	bool painted;
	bool broken;
	uint32_t spawned_count;
};

struct Material {
	uint8_t kind; // MaterialKind
	Color rgba;
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;	// [0..32]
	bool is_tint;
};

struct Palette {
	Material materials[256];
	bool z_u8;
	uint8_t black_tint[4 * 256];
	uint8_t yellow_tint[4 * 256];
	uint8_t rgba_tint[4 * 256];
};

struct Unk1 {
	Vector z_1;
	Vector z_2;
	float z_3;
	float z_4;
	uint32_t z_5;
	float z_6;
};

struct Unk2 {
	string z_1;
	bool z_2;
};

struct Scene {
	char magic[5];				// TDBIN
	uint8_t version[3];			// version
	string level_id;
	string level_path;
	string layers;
	string mod;

	uint32_t aaa1;
	Vec<Registry> enabled_mods;
	Vec<Registry> spawned_mods;

	uint32_t driven_vehicle;	// driven
	Vector shadow_volume;		// shadowVolume
	Transform spawnpoint;		// spawnpoint
	uint32_t world_body;
	uint32_t flashlight;
	uint32_t explosion_lua;
	uint32_t achievements_lua;
	PostProcessing postpro;
	Player player;
	Environment environment;
	Boundary boundary;
	Vec<Fire> fires;
	Vec<Palette> palettes;
	Vec<Registry> registry;
	Vec<Entity*> entities;
	uint32_t entity_count;
	uint32_t broken_voxels;
	Vec<Unk1> z_st1;
	bool has_snow;
	Vec<Unk2> z_st2;
};

#endif
