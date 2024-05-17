#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stdexcept>
#include <string>

#include "lua_table.h"
#include "math_utils.h"

using namespace std;

template <typename T>
class Vec {
private:
	uint32_t size;
	T* data;
public:
	Vec() {
		size = 0;
		data = nullptr;
	}
	void resize(uint32_t size) {
		this->size = size;
		if (size > 0)
			data = new T[size];
	}
	~Vec() {
		size = 0;
		delete[] data;
		data = nullptr;
	}
	uint32_t getSize() const {
		return size;
	}
	Vec& operator=(const Vec& other) {
		if (this != &other) {
			resize(other.getSize());
			for (unsigned int i = 0; i < size; i++)
				data[i] = other[i];
		}
		return *this;
	}
	T operator[](uint32_t index) const {
		if (index > size - 1)
			throw out_of_range("Position out of range");
		return data[index];
	}
	T& operator[](uint32_t index) {
		if (index > size - 1)
			throw out_of_range("Position out of range");
		return data[index];
	}
};

#define SmallVec Vec

extern const char* EntityKindName[];

struct Vertex {		// vertex
	float pos[2];	// pos
};

struct Registry {
	string key;
	string value;
};

struct Tag {
	string name;
	string value;
};

struct Color {
	float r, g, b, a;
};

struct Sound {
	string path;
	float volume;
};

// ------------------------------------

enum EntityKind { // uint8_t
	KindBody = 1,
	KindShape,
	KindLight,
	KindLocation,
	KindWater,
	KindJoint,
	KindVehicle,
	KindWheel,
	KindScreen,
	KindTrigger,
	KindScript,
};

struct Entity {
	uint8_t type;
	uint32_t handle;
	SmallVec<Tag> tags;
	string desc;		// desc

	uint16_t flags;
	void* self;

	Vec<Entity*> children;
	uint32_t beef_beef;

	Entity* parent;		// helper for graph navigation
	~Entity();
};

struct Body {
	Transform transform;
	Vector velocity;
	Vector angular_velocity;
	bool dynamic;				// dynamic
	uint8_t active;
	float friction;
	uint8_t friction_mode;
	float restitution;
	uint8_t restitution_mode;
};

struct Voxels {
	uint32_t size[3];
	// if the shape volume is not empty, voxels are stored using run length encoding
	// with pairs (n-1, i) in xyz order
	RLE palette_indexes;
};

struct Shape {
	Transform transform;
	uint16_t shape_flags;		// collide: 0x10
	uint8_t collision_layer;
	uint8_t collision_mask;
	float density;				// density
	float strength;				// strength
	uint16_t texture_tile;		// texture
	uint16_t blendtexture_tile;	// blendtexture
	float texture_weight;		// texture
	float blendtexture_weight;	// blendtexture
	Vector texture_offset;
	float emissive_scale;
	bool is_broken;
	uint8_t has_voxels;

	Voxels voxels;
	uint32_t palette;
	float scale;				// scale = 10.0 * this
	// 0xFFFFFFFF 0xFFFFFFFF
	uint32_t z_u32_2[2]; // 8 bytes
	bool is_disconnected;

	uint8_t z3_u8;

	Transform old_transform;		// helper for screen positon
};

enum LightType { // uint8_t
	Sphere = 1,
	Capsule,
	Cone,
	Area,
};

struct Light {
	bool is_on;
	uint8_t type;		// type
	Transform transform;
	Color color;		// color = pow(this, 1 / 2.2)
	float scale;		// scale
	float reach;		// reach
	float size;			// size
	float unshadowed;	// unshadowed
	float angle;		// angle = 2.0 * deg(acos(this))
	float penumbra;		// penumbra = 2.0 * deg(acos(angle) - acos(this))
	float fogiter;		// fogiter
	float fogscale;		// fogscale
	float area_size[2];	// size = 2.0 * this
	float capsule_size;	// size.x = 2.0 * this
	Vector position;
	uint8_t index;
	float z_f32;
	Sound sound;		// sound
	float glare;		// glare
};

struct Location {
	Transform transform;
};

struct Water {
	Transform transform;
	float depth;		// depth
	float wave;			// wave
	float ripple;		// ripple
	float motion;		// motion
	float foam;			// foam
	Color color; 		// color
	float visibility;	// visibility
	Vec<Vertex> water_vertices;
};

enum JointType { // uint32_t
	Ball = 1,
	Hinge,
	Prismatic,
	_Rope,
};

struct Knot {
	Vector from;
	Vector to;
};

struct Rope {
	Color color;		// color
	float z_f32;
	float strength;		// strength
	float maxstretch;	// maxstretch
	float slack;		// slack
	float z2_f32;
	uint8_t z_u8;
	Vec<Knot> knots;
};

struct Joint {
	uint32_t type;				// type
	uint32_t shape_handles[2];	// Conected shapes handles
	Vector shape_positions[2];	// Relative position to each shape
	Vector shape_axes[2];		// Last column of YZX rotation matrix
	bool connected;
	bool collide;				// collide
	float rotstrength;			// rotstrength
	float rotspring;			// rotspring
	float ball_rot[4];
	float limits[2];			// limits (in degrees for hinge, meters for prismatic)
	float z_f32_2[2];
	float size;					// size
	bool sound;					// sound
	bool autodisable;			// autodisable
	float z_f32_4[2];
	Rope rope;					// Only if type = Rope
};

struct VehicleSound {
	string name;
	float pitch;
};

struct VehicleProperties {
	float topspeed;		// topspeed = 3.6 * this, in m/s
	float top_speed_clamp;	// clamped to 30 Km/h, in m/s
	float spring;		// spring
	float damping;		// damping
	float acceleration;	// acceleration
	float strength;		// strength
	float friction;		// friction
	float max_steering_angle; // in rad, always 30°
	bool handbrake;
	float antispin;		// antispin
	float steerassist;	// steerassist
	float z_f32;		// TODO: 1.5 ???
	float antiroll;		// antiroll
	VehicleSound sound;	// sound
};

struct Exhaust {
	Transform transform;
	float strength;
};

struct Vital {
	uint32_t body_handle;
	Vector pos;
	float z_f32;		// TODO: 0.5 ???
	uint32_t shape_handle;
};

struct Vehicle {
	uint32_t body_handle;
	Transform transform;
	Transform transform2;
	Vec<uint32_t> wheel_handles;
	VehicleProperties properties;
	Vector camera;			// camera
	Vector player;			// player
	Vector exit;			// exit
	Vector propeller;		// propeller
	float difflock;			// difflock
	float health;
	uint32_t main_voxel_count;
	bool breaking;
	float z1_f32;
	Vec<uint32_t> refs;
	Vec<Exhaust> exhausts;	// exhaust
	Vec<Vital> vitals;		// vital
	float bounds_dist;
	bool noroll;
	float brokenthreshold;
	float z2_f32;
};

struct Wheel {
	uint32_t vehicle;
	uint32_t vehicle_body;
	uint32_t body;
	uint32_t shape;
	uint32_t z_handle;
	uint32_t z1_f32_3[3];
	uint8_t z_u8;
	Transform transform;
	Transform empty_transform;
	float steer;			// steer
	float drive;			// drive
	float travel[2];		// travel
	float radius;
	float width;
	float angular_speed;
	float z2_f32_2[2];
};

struct Screen {
	Transform transform;
	float size[2];			// size
	float bulge;			// bulge
	uint32_t resolution[2];	// resolution
	string script;			// script
	bool enabled;			// enabled
	bool interactive;		// interactive
	float emissive;			// emissive
	float fxraster;			// fxraster
	float fxca;				// fxca
	float fxnoise;			// fxnoise
	float fxglitch;			// fxglitch
};

enum TriggerKind { // uint32_t
	TrSphere = 1,
	TrBox,
	TrPolygon,
};

struct TriggerSound {
	string path;		// sound
	float soundramp;	// soundramp
	uint8_t z_u8;
	float volume;		// sound
};

struct Trigger {
	Transform transform;
	uint32_t type;				// type
	float sphere_size;			// size
	float box_size[3];			// size = 2.0 * this
	float polygon_size;			// size
	Vec<Vertex> polygon_vertices;
	TriggerSound sound;
};

struct ScriptSound {
	uint32_t kind;
	string name;
};

struct ValueTransition {
	string variable;
	uint8_t kind;
	float transition_time;
	float time;
	float z1_f32;
	float z2_f32;
};

struct Script {
	string file;			// file
	Vec<Registry> params;	// param%d
	float last_update;
	float time;
	uint32_t z_u32;
	LuaTable table;
	Vec<uint32_t> entity_handles;
	Vec<ScriptSound> sounds;
	Vec<ValueTransition> transitions;
};

#endif
