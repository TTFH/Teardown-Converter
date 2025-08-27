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

#define VERSION_1_7_0 170
#define VERSION_1_6_3 163
#define VERSION_1_6_0 160
#define VERSION_1_5_4 154
#define LAST_VERSION VERSION_1_7_0

#define SmallVec Vec
//typedef uint32_t Handle;

extern const char* EntityName[];

struct Vertex {	// vertex
	float x, y;	// pos
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
/*
enum Flags : uint16_t {
	NoShadow	= 1 << 12,
	Invisible	= 1 << 9,
	Interact	= 1 << 8,
	NoNav		= 1 << 7,
	AutoBreak	= 1 << 6,
	BreakAll	= 1 << 5,
	NoCull		= 1 << 4,
	Debris?		= 1 << 3,
	Unbreakable	= 1 << 2,
	InheritTags	= 1 << 1,
	HasTag		= 1 << 0,
};
*/
struct Entity {
	static const uint8_t Body = 1;
	static const uint8_t Shape = 2;
	static const uint8_t Light = 3;
	static const uint8_t Location = 4;
	static const uint8_t Water = 5;
	static const uint8_t Joint = 6;
	static const uint8_t Vehicle = 7;
	static const uint8_t Wheel = 8;
	static const uint8_t Screen = 9;
	static const uint8_t Trigger = 10;
	static const uint8_t Script = 11;
	static const uint8_t Animator = 12;

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
/*
enum BodyMode : uint8_t {
	Average = 0,
	Minimum = 1,
	Multiply = 2,
	Maximum = 3,
};
*/
struct Body {
	Transform transform;
	Vec3 velocity;
	Vec3 angular_velocity;
	bool dynamic;				// dynamic
	uint8_t active;
	float friction;
	uint8_t friction_mode;
	float restitution;
	uint8_t restitution_mode;
};
/*
enum ShapeFlags {
	Visible		= 1 << 5,
	Physical	= 1 << 4,
	Small		= 1 << 3,
	Large		= 1 << 2,
	Dynamic		= 1 << 1,
	Static		= 1 << 0,
};

enum ShapeOrigin : uint8_t {
	Tool = 1,
	MapInit = 2,
	Debris = 3,
	CreateShape = 4,
	Spawn = 5,
};
*/
struct Voxels {
	uint32_t size[3];
	// if the shape volume is not empty, voxels are stored using run length encoding
	// with pairs (n-1, i) in xyz order
	RLE palette_indexes;
	uint32_t palette;
	float scale;				// scale = 10.0 * this
	uint8_t light_mask[8];		// - - - - | f r b -
	bool is_disconnected;
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
	Vec3 texture_offset;
	float emissive_scale;
	bool is_broken;
	uint8_t has_voxels;
	Voxels voxels;
	uint8_t origin;
	uint32_t animator;

	Transform old_transform;	// helper for screen positon
};

struct Light {
	static const uint8_t Sphere = 1;
	static const uint8_t Capsule = 2;
	static const uint8_t Cone = 3;
	static const uint8_t Area = 4;

	bool enabled;
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
	Vec3 position;
	uint8_t index;
	float flickering;
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
	Vec<Vertex> vertices;
};

struct Segment {
	Vec3 from;
	Vec3 to;
};

struct Rope {
	Color color;		// color
	float zero;
	float strength;		// strength
	float maxstretch;	// maxstretch
	float slack;		// slack
	float segment_length;
	uint8_t active;
	Vec<Segment> segments;
};

struct Joint {
	static const uint32_t Ball = 1;
	static const uint32_t Hinge = 2;
	static const uint32_t Prismatic = 3;
	static const uint32_t _Rope = 4;
	static const uint32_t Cone = 5;

	uint32_t type;				// type
	uint32_t shapes[2];
	Vec3 positions[2];
	Vec3 axes[2];
	bool connected;
	bool collide;				// collide
	float rotstrength;			// rotstrength
	float rotspring;			// rotspring
	Quat hinge_rot;
	float limits[2];			// limits (in degrees for hinge, meters for prismatic)
	float max_velocity;
	float strength;
	float size;					// size
	bool sound;					// sound
	bool autodisable;			// autodisable
	float connection_strength;	// Used for planks 3000.0
	float disconnect_dist;		// Used for planks 0.8
	Rope* rope;
	~Joint();
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
	float max_steer_angle; // in rad, always 30°
	bool handbrake;
	float antispin;		// antispin
	float steerassist;	// steerassist
	float assist_multiplier;
	float antiroll;		// antiroll
	VehicleSound sound;	// sound
};

struct Exhaust {
	Transform transform;
	float strength;
};

struct Vital {
	uint32_t body;
	Vec3 position;
	float radius;
	uint32_t nearby_voxels;
};

struct VehicleLocation {
	string name;
	Transform transform;
	uint32_t handle;
};

struct Vehicle {
	uint32_t body;
	Transform transform;
	Transform transform2;
	Vec<uint32_t> wheels;
	VehicleProperties properties;
	Vec3 camera;			// camera
	Vec3 player;			// player
	Vec3 exit;			// exit
	Vec3 propeller;		// propeller
	float difflock;			// difflock
	float health;
	uint32_t main_voxel_count;
	bool braking;
	float passive_brake;
	Vec<uint32_t> bodies;
	Vec<Exhaust> exhausts;	// exhaust
	Vec<Vital> vitals;		// vital
	Vec<VehicleLocation> locations;
	float bounds_dist;
	bool noroll;
	float brokenthreshold;
	float smokeintensity;
};

struct Wheel {
	uint32_t vehicle;
	uint32_t vehicle_body;
	uint32_t body;
	uint32_t shape;
	uint32_t ground_shape;
	uint32_t ground_voxel_pos[3];
	bool on_ground;
	Transform transform;	// in the vehicle body local space
	Transform transform2;	// in the wheel body local space
	float steer;			// steer
	float drive;			// drive
	float travel[2];		// travel
	float radius;
	float width;
	float angular_speed;
	float stance;			// wheel width * side (-1, 0, 1), right is positive
	float vertical_offset;	// vertical distance to vehicle body, down is positive
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
/*
enum TriggerSoundType : uint8_t {
	Default = 0,
	Outdoor = 1,
	Music = 2
};
*/
struct TriggerSound {
	string path;		// sound
	float ramp;			// soundramp
	uint8_t type;
	float volume;		// sound
};

struct Trigger {
	static const uint32_t Sphere = 1;
	static const uint32_t Box = 2;
	static const uint32_t Polygon = 3;

	Transform transform;
	uint32_t type;				// type
	float sphere_size;			// size
	float box_size[3];			// size = 2.0 * this
	float polygon_size;			// size
	Vec<Vertex> polygon_vertices;
	TriggerSound sound;
};
/*
enum SoundType : uint32_t {
	Sound = 1,
	Loop = 2,
	//Sprite = 3, :skull:
	Music = ?,
	UiSound = ?,
	UiLoop = ?
};
*/
struct ScriptSound {
	uint32_t type;
	string name;
};
/*
enum TransitionType : uint8_t {
	Linear = 1,
	EaseIn = 2,
	EaseOut = 3,
	Cosine = 4,
	Bounce = 5
};
*/
struct ValueTransition {
	string variable;
	uint8_t transition;
	float target_time;
	float current_time;
	float current_value;
	float target_value;
};

struct Script {
	string file;			// file
	Vec<Registry> params;	// param%d
	float tick_time;
	float update_time;
	uint32_t variables_count;
	LuaTable* variables;
	Vec<uint32_t> entities;
	Vec<ScriptSound> sounds;
	Vec<ValueTransition> transitions;
	~Script();
};
/*
struct Bone {
	uint32_t body;
	int index;
	int parent_index;
	string name;
};

struct Keyframe {
	Transform transform;
	float time;
};

struct Sequence {
	uint32_t bone_index;
	Vec<Keyframe> frames;
};

struct Animation {
	string name;
	uint32_t start_bone;
	float anim_rate;
	Vec<Sequence> seqs;
};
*/
struct Animator {
	Transform transform;
	string path;		// file
	// ...
};

#endif
