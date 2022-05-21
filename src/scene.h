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
	float velocity[3];
	float health;
	float z_f32_4[4];
};

struct Sun {
	float tint_brightness[3];
	Rgba colorTint;		// sunColorTint
	float dir[3];		// sunDir
	float brightness;	// sunBrightness
	float spread;		// sunSpread
	float length;		// sunLength
	float fogScale;		// sunFogScale
	float glare;		// sunGlare
};

struct Skybox {
	string texture;			// skybox
	Rgba tint;				// skyboxtint
	float brightness;		// skyboxbrightness
	float rot;				// skyboxrot in radians
	Sun sun;
	uint8_t z_u8;
	Rgba constant;			// constant
	float ambient;			// ambient
	float ambientexponent;	// ambientexponent
};

struct Fog {
	Rgba color;			// fogColor
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
	float dir[3];	// snowdir x y z
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
	float wind[3];		// wind
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
	float pos[3];
	float max_time;
	float time;
	uint8_t z_u8_6[6];
};

struct Material {
	uint8_t kind; // MaterialKind
	Rgba rgba;
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;	// [0..32]
	bool replacable;
};

struct Palette {
	Material materials[256];
	uint8_t tint_tables[2 * 4 * 256];
	uint8_t z_u8;
};

struct PostProcessing {
	float brightness;		// brightness
	Rgba colorbalance;		// colorbalance
	float saturation;		// saturation
	float gamma;			// gamma
	float bloom;			// bloom
};

struct Scene {
	uint8_t magic[5];		// TDBIN
	uint8_t version[3];		// version
	string level;
	uint32_t driven_vehicle;// driven
	float shadowVolume[3];	// shadowVolume
	Transform spawnpoint;	// spawnpoint
	uint32_t z_u32_3[3];
	PostProcessing postpro;
	Player player;
	Environment environment;
	Boundary boundary;
	Vec<Fire> fires;
	Vec<Palette> palettes;
	Vec<Registry> registry;
	Vec<Entity*> entities;
};

#endif
