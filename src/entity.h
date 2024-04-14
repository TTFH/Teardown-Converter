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

#define VERSION_1_5_4 154 // Changes to boundary, shape, vehicle and body
#define VERSION_1_5_0 150 // Added scene path, list of enabled mods, extra paint color
#define VERSION_1_4_0 140 // Added water->visibility
#define VERSION_1_3_0 130 // Added 17 bytes to Wheel
#define VERSION_1_2_0 120 // No changes
#define VERSION_1_1_0 110 // Added handle to achievements.lua
#define VERSION_1_0_0 100 // No changes
#define VERSION_0_9_6 96 // Removed ~~Herobrine~~ Enemy spiders
#define VERSION_0_9_5 95 // Added joint->autodisable
#define VERSION_0_9_2 92 // Entity flags increased to 16 bits
#define VERSION_0_9_0 90 // Added Boundary padding, Water color, blend texture, joint sound, etc.
#define VERSION_0_8_0 80 // Added skybox->brightness, postprocessing->bloom
#define VERSION_0_7_4 74 // Added var transitions to Script for SetValue()
#define VERSION_0_7_2 72 // No changes
#define VERSION_0_7_1 71 // Added stuff to env, probably snow related
#define VERSION_0_7_0 70 // Added PostProcessing, Capsule light, vehicle->exhausts[].strength, more stuff to env, explosion.lua
#define VERSION_0_6_2 62 // No changes
#define VERSION_0_6_1 61 // No changes
#define VERSION_0_5_5 55 // Added env->slippery
#define VERSION_0_5_2 52 // No changes
#define VERSION_0_5_1 51 // Added level name, flashlight and world body handles
#define VERSION_0_5_0 50 // Added light->scale, removed list from Script (order of childrens?)
#define VERSION_0_4_6 46 // No changes
#define VERSION_0_4_5 45 // Added magic, version, fog->color
#define VERSION_0_3_0 30 // Perf test

#define SmallVec Vec

typedef uint16_t EntityFlags;

extern const char* EntityKindName[];

struct Vertex { // vertex
	float pos[2]; // pos
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

struct MeshVertex {
	Vector position;
	Vector normal;
};

// ------------------------------------

enum EntityKind { // uint8_t
	KindBody = 1,
	KindShape,
	KindLight,
	KindLocation,
	KindWater,
	KindEnemy,
	KindJoint,
	KindVehicle,
	KindWheel,
	KindScreen,
	KindTrigger,
	KindScript,
};

struct Entity {
	uint8_t kind_byte;
	uint32_t handle;
	SmallVec<Tag> tags; // uint8_t size
	string desc;		// desc
	void* kind;
	Vec<Entity*> children;
	uint32_t beef_beef;

	Entity* parent;
	~Entity();
};

struct Body {
	EntityFlags flags;
	Transform transform;
	Vector velocity;
	Vector angular_velocity;
	bool dynamic;				// dynamic
	uint8_t body_flags;
	float z1_f32;
	uint8_t z1_u8;
	float z2_f32;
	uint8_t z2_u8;
};

struct Voxels {
	uint32_t size[3];
	// if the shape volume is not empty, voxels are stored using run length encoding
	// with pairs (n-1, i) in xyz order
	RLE palette_indexes;
};

struct Shape {
	EntityFlags flags;
	Transform transform;
	uint16_t shape_flags;			// 0x10 = collide
	uint8_t collision_layer;
	uint8_t collision_mask;
	float density;					// density
	float strength;					// strength
	uint16_t texture_tile;			// texture
	uint16_t blendtexture_tile = 0;	// blendtexture
	float texture_weight;			// texture
	float blendtexture_weight = 1;	// blendtexture
	Vector texture_offset;
	float emissive_scale;
	uint8_t z1_u8;
	uint8_t shape_type;
	Voxels voxels;
	uint32_t palette;
	float scale;					// scale = 10.0 * this
	// 0xFFFFFFFF 0xFFFFFFFF 0x00
	uint32_t z_u32_2[2];
	uint8_t z3_u8;
	uint8_t z4_u8;

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
	float scale = 1;	// scale
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
	EntityFlags flags;
	Transform transform;
};

struct Water {
	EntityFlags flags;
	Transform transform;
	float depth;				// depth
	float wave;					// wave
	float ripple;				// ripple
	float motion;				// motion
	float foam;					// foam
	Color color = { 0.01, 0.01, 0.01, 1 }; // color
	float visibility = 3;		// visibility
	Vec<Vertex> water_vertices;
};

struct MeshShapeCommon {
	uint8_t z_u8; // if child
	uint32_t handle;
	uint8_t z_u8_3[3];
	Transform transform;
	float z_f32_8[8];
};

struct MeshShape {
	MeshShapeCommon data;
	float z_f32;
	uint8_t z_u8_3[3];
	Color color;
	float z_f32_4[4];
	Vec<MeshVertex> vertices;
	Vec<uint32_t> indices;
	uint32_t z_u32;
	uint32_t beef_beef;
};

struct Enemy {
	uint8_t z_u8;
	uint32_t z_u32;
	MeshShapeCommon data;
	MeshShape shape;
	MeshShape child_shapes[27];
	uint32_t beef_beef;
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
	float z_f32_2[2];			// Unknown, for hinge maybe???
	float size;					// size
	bool sound;					// sound
	bool autodisable;			// autodisable
	uint32_t z_u32_2[2];
	Rope rope;					// Only if type = Rope
};

struct VehicleSound {
	string name;
	float pitch;
};

struct VehicleProperties {
	float topspeed;		// topspeed = 3.6 * this
	float z1_f32;		// mostly 8.3334 ???
	float spring;		// spring
	float damping;		// damping
	float acceleration;	// acceleration
	float strength;		// strength
	float friction;		// friction
	float max_steering_angle; // in rad, always 30°
	bool handbrake;
	float antispin;		// antispin
	float steerassist;	// steerassist
	float z3_f32;		// 1.5 ???
	float antiroll;		// antiroll
	VehicleSound sound;	// sound
	// TODO: smokeintensity
};

struct Exhaust {
	Transform transform;
	float strength = 1;
};

struct Vital {
	uint32_t body_handle;
	Vector pos;
	float z_f32;		// 0.5 ???
	uint32_t shape_handle;
};

struct Vehicle {
	EntityFlags flags;
	uint32_t body_handle;
	Transform transform;
	Vector velocity;
	Vector angular_velocity;
	float z1_f32;
	Vec<uint32_t> wheel_handles;
	VehicleProperties properties;
	Vector camera;			// camera
	Vector player;			// player
	Vector exit;			// exit
	Vector propeller;		// propeller
	float difflock;			// difflock
	float boat_sink;
	uint32_t main_voxel_count;
	uint8_t z1_u8;
	float drivable;			// 0 if nodrive
	Vec<uint32_t> refs;
	Vec<Exhaust> exhausts;	// exhaust
	Vec<Vital> vitals;		// vital
	float z2_f32;
	uint8_t z2_u8;
	float brokenthreshold;
	float z3_f32;
};

struct Wheel {
	EntityFlags flags;
	uint32_t vehicle;
	uint32_t vehicle_body;
	uint32_t body;
	uint32_t shape;
	uint32_t z1_f32_4[4];
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
	EntityFlags flags;
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
	EntityFlags flags;
	Transform transform;
	uint32_t type;				// type
	float sphere_size;			// size
	float box_size[3];			// size = 2.0 * this
	float polygon_size;			// size
	Vec<Vertex> polygon_vertices;
	TriggerSound sound;
};

enum ScriptSoundKind { // uint32_t
	Normal = 1,
	Loop,
	Unknown,
};
/*
enum TransitionKind { // uint8_t
	Cosine,
	EaseIn,
	EaseOut,
	Bounce,
	Linear
};
*/
struct ScriptSound {
	uint32_t kind;
	string name;
};

struct ValueTransition {
	string variable;
	uint8_t kind;
	float transition_time;
	double time;
	uint8_t z_u8_4[4];
};

struct Script {
	EntityFlags flags;
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
