#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>
#include <string>

#include "entity.h"

using namespace std;

extern const char* MaterialName[];
extern const char* FogType[];

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
	float bluetide_timer;	// game.player.steroid
	float bluetide_power;
	uint32_t animator;
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
	float rot;				// skyboxrot (in rad)
	Sun sun;
	bool auto_sun_dir;
	Color constant;			// constant
	float ambient;			// ambient
	float ambientexponent;	// ambientexponent
};

struct Fog {
	uint8_t type;
	Color color;	// fogColor
	// fogParams:
	float start;
	float distance;	// end = start + distance
	float amount;
	float exponent;
	float height_offset;
};

struct Snow {
	Vector dir;		// snowdir x y z
	float spread;	// snowdir spread
	float amount;	// snowamount
	float speed;	// snowamount
	bool onground;	// snowonground
};

struct Environment {
	struct Water {
		float wetness;		// wetness
		float puddleamount;	// puddleamount
		float puddlesize;	// puddlesize = 0.01 / this
		float rain;			// rain
	};

	Skybox skybox;
	float exposure[2];	// exposure
	float brightness;	// brightness
	Fog fog;
	Water water;
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
	float padleft;		// -padleft
	float padtop;		// -padtop
	float padright;		// padright
	float padbottom;	// padbottom
	float maxheight;	// maxheight
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
	static const uint8_t None = 0;
	static const uint8_t Glass = 1;
	static const uint8_t Wood = 2;
	static const uint8_t Masonry = 3;
	static const uint8_t Plaster = 4;
	static const uint8_t Metal = 5;
	static const uint8_t HeavyMetal = 6;
	static const uint8_t Rock = 7;
	static const uint8_t Dirt = 8;
	static const uint8_t Foliage = 9;
	static const uint8_t Plastic = 10;
	static const uint8_t HardMetal = 11;
	static const uint8_t HardMasonry = 12;
	static const uint8_t Ice = 13;
	static const uint8_t Unphysical = 14;

	uint8_t type;
	Color rgba;
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;	// [0..32]
	bool is_tint;
};

struct Palette {
	Material materials[256];
	bool has_transparent;
	uint8_t black_tint[4 * 256];
	uint8_t yellow_tint[4 * 256];
	uint8_t rgba_tint[4 * 256];
};
/*
enum ProjectileType : uint32_t {
	Gun = 0,
	Rocket = 2,
	??? = 4,
	Bullet = 5,
	// Shotgun
};
*/
struct Projectile {
	Vector origin;
	Vector direction;
	float dist;
	float max_dist;
	uint32_t type;
	float strength;
};

struct Path {
	string folder;
	bool do_override;
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
	uint32_t characters_lua;

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
	Vec<Projectile> projectiles;
	bool has_snow;
	Vec<Path> assets;
};

#endif
