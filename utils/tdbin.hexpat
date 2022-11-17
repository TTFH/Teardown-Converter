#include <std/io.pat>
#include <std/mem.pat>
#include <std/sys.pat>
#include <std/math.pat>

#pragma pattern_limit 131072
#pragma array_limit 65536

/*
	RELEASE DATE		MANIFEST ID			VERSION
2 November 2022 	9007227657098510758		1.2.0
21 October 2022 	5037891319466838476		1.2.0
16 June 2022 		7205895291629880249		1.1.0
21 April 2022 		3736615329123538866		1.0.0
5 April 2022 		7537052615747297911		0.9.5
17 March 2022 		7467090028228665435		0.9.5
1 March 2022 		8184114032844459263		0.9.5
20 December 2021	6815317348286257307		0.9.2
2 December 2021 	7659409681954156608		0.9.0
7 October 2021 		1380852899307030446		0.8.0
28 June 2021 		7593428965132473006		0.7.4
11 May 2021 		1576673398223395400		0.7.2
4 March 2021 		7508456906693476405		0.6.2
15 February 2021	3922109535515865206		0.5.5
5 February 2021 	2728955346983677370		0.5.5
22 December 2020	1454856181527574565		0.5.2
5 November 2020 	8772779893907758192		0.4.6
29 October 2020 	 755392188540739651		0.4.5
~24 September 2020			  Perf test		0.3.0
*/

using BitFlags = u16;

struct SmallVec<T> {
    u8 size [[hidden]];
    T data[size] [[inline]];
};

struct Vec<T> {
    u32 size [[hidden]];
    T data[size] [[inline]];
};

struct Vector {
	float x, y, z;
} [[static, sealed, format("format_vec")]];

fn format_vec(ref auto vec) {
	return std::format("Vec({}, {}, {})", vec.x, vec.y, vec.z);
};

struct Quat {
	float x, y, z, w;
} [[static]];

struct Transform {
	Vector pos;
	Quat rot;
} [[static, sealed, format("format_tr")]];

fn format_tr(ref auto tr) {
	return std::format("Trasform(Vec({}, {}, {}), Quat({}, {}, {}, {}))", tr.pos.x, tr.pos.y, tr.pos.z, tr.rot.x, tr.rot.y, tr.rot.z, tr.rot.w);
};

struct Color {
	float r, g, b, a;
} [[static, sealed, format("format_color")]];

fn format_color(ref auto color) {
	return std::format("rgba({}, {}, {}, {})", color.r, color.g, color.b, color.a);
};

struct PostProcessing {
	float brightness;		// brightness
	Color colorbalance;		// colorbalance
	float saturation;		// saturation
	float gamma;			// gamma
	float bloom;			// bloom
} [[static, single_color]];

struct Player {
	Transform transform;
	float yaw;
	float pitch;
	Vector velocity;
	float health;
	float z_f32_4[4] [[hidden]];
} [[static, single_color]];

struct Sun {
	float tint_brightness[3];
	Color colorTint;	// sunColorTint
	Vector dir;			// sunDir
	float brightness;	// sunBrightness
	float spread;		// sunSpread
	float length;		// sunLength
	float fogScale;		// sunFogScale
	float glare;		// sunGlare
} [[static]];

struct Skybox {
	char texture[];			// skybox
	Color tint;				// skyboxtint
	float brightness;		// skyboxbrightness
	float rot;				// skyboxrot in radians
	Sun sun;				// sun
	u8 z_u8;
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
} [[static]];

struct EnvWater {
	float wetness;		// wetness
	float puddleamount;	// puddleamount
	float puddlesize;	// puddlesize =  0.01 / this
	float rain;			// rain
} [[static]];

struct Snow {
	Vector dir;		// snowdir
	float spread;	// snowdir spread
	float amount;	// snowamount
	float speed;
	bool onground;	// snowonground
} [[static]];

struct Sound {
	char path[];
	float volume;
};

struct Environment {
	Skybox skybox;		// skybox
	float exposure[2];	// exposure
	float brightness;	// brightness
	Fog fog;			// fog
	EnvWater water;		// water
	bool nightlight;	// nightlight
	Sound ambience;		// ambience
	float slippery;		// slippery
	float fogscale;		// fogscale
	Snow snow;			// snow
	Vector wind;		// wind
	float waterhurt;	// waterhurt
} [[single_color]];

struct Fire {
	u32 entity_handle;
	Vector pos;
	float max_time;
	float time;
	u8 z_u8_6[6];
} [[static]];

struct Vertex { // vertex
	float pos[2]; // pos
} [[static]];

struct Boundary {
	Vec<Vertex> vertices;
	float padleft;		// -padleft
	float padtop;		// -padtop
	float padright;		// padright
	float padbottom;	// padbottom
} [[single_color]];

enum MaterialKind : u8 {
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

struct Material {
	MaterialKind kind;
	Color Color;
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;
	bool replaceable;
} [[static]];

struct Palette {
	Material materials[256];
	u8 z_u8;
	u8 tint_table[2 * 4 * 256]; // Black | Yellow
} [[static, sealed, single_color]];

struct Registry {
	char key[];
	char value[];
};

struct Tag {
	char name[];
	char value[];
};

enum EntityKind : u8 {
	Body = 1,
	Shape,
	Light,
	Location,
	Water,
	Joint,
	Vehicle,
	Wheel,
	Screen,
	Trigger,
	Script,
};

struct Body {
	BitFlags flags;
	Transform transform;
	Vector velocity;
	Vector angular_velocity;
	bool dynamic;			// dynamic
	u8 body_flags;
} [[static]];

struct Voxels {
	u32 sizex;
	u32 sizey;
	u32 sizez;
	if (sizex * sizey * sizez > 0)
		Vec<u8> palette_index;
};

struct Shape {
	BitFlags flags;
	Transform transform;
	u8 shape_flags;				// 0x10 = collide
	u8 z_u8_3[3];
	float density;				// density
	float strength;				// strength
	u16 texture_tile;			// texture
	u16 blendtexture_tile;		// blendtexture
	float texture_weight;		// texture
	float blendtexture_weight;	// blendtexture
	Vector starting_world_position;
	float z_f32;
	u8 z1_u8;
	u8 z2_u8;
	Voxels voxels;				// voxels
	u32 palette;				// palette
	float scale;				// scale = 10.0 * this
	u32 z_u32_2[2];
	u8 z3_u8;
};

struct Light {
	bool is_on;
	u8 type;			// type
	Transform transform;
	Color color;			// color = pow(this, 0.454545)
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
	u8 z_u8_13[13];
	float z_f32;
	Sound sound;		// sound
	float glare;		// glare
} [[static]];

struct Location {
	BitFlags flags;
	Transform transform;
} [[static]];

struct Water {
	BitFlags flags;
	Transform transform;
	float depth;		// depth
	float wave;			// wave
	float ripple;		// ripple
	float motion;		// motion
	float foam;			// foam
	Color color;			// color
	Vec<Vertex> vertices; // vertices
};

struct Knot {
	Vector from;
	Vector to;
} [[static]];

struct Rope {
	Color color;			// color
	float z_f32;
	float strength;		// strength
	float maxstretch;	// maxstretch
	float slack;		// slack = this - length
	float z2_f32;
	u8 z_u8;
	Vec<Knot> knots;	// location / length
};

enum JointType : u32 {
	Ball = 1,
	Hinge,
	Prismatic,
	Rope,
};

struct Joint {
	JointType type;				// type
	u32 shape_handles[2];		// Conected shapes handles
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
	u32 z_u32_2[2];
	if (type == JointType::Rope)
		Rope rope;
};

struct VehicleSound {
	char name[];
	float pitch;
};

struct VehicleProperties {
	float topspeed;		// topspeed = 3.6 * this
	float z1_f32;
	float spring;		// spring
	float damping;		// damping
	float acceleration;	// acceleration
	float strength;		// strength
	float friction;		// friction
	float z2_f32;
	u8 z_u8;
	float antispin;		// antispin
	float steerassist;	// steerassist
	float z3_f32;
	float antiroll;		// antiroll
	VehicleSound sound;	// sound
};

struct Exhaust {
	Transform transform;
	float strength;
} [[static]];

struct Vital {
	u32 body_handle;
	Vector pos;
	float z_f32;
	u32 shape_index;
} [[static]];

struct Vehicle {
	BitFlags flags;
	u32 body_handle;
	Transform transform;
	Vector velocity;
	Vector angular_velocity;
	float z1_f32;
	Vec<u32> wheel_handles;
	VehicleProperties properties;
	Vector camera;			// camera
	Vector player;			// player
	Vector exit;			// exit
	Vector propeller;		// propeller
	float difflock;			// difflock
	float z2_f32;
	u32 z_u32;
	u8 z1_u8;
	float z3_f32;
	Vec<u32> refs;
	Vec<Exhaust> exhausts;	// exhaust
	Vec<Vital> vitals;		// vital
	float z4_f32;
	u8 z2_u8;
	float z5_f32;
};

struct Wheel {
	BitFlags flags;
	u32 vehicle;
	u32 vehicle_body;
	u32 body;
	u32 shape;
	Transform transform;
	float z_f32_7[7];
	float steer;			// steer
	float drive;			// drive
	float travel[2];		// travel
	float z1_f32_2[2];
	float angular_speed;
	float z2_f32_2[2];
} [[static]];

struct Screen {
	BitFlags flags;
	Transform transform;
	float size[2];			// size
	float bulge;			// bulge
	u32 resolution[2];		// resolution
	char script[];			// script
	bool enabled;			// enabled
	bool interactive;		// interactive
	float emissive;			// emissive
	float fxraster;			// fxraster
	float fxca;				// fxca
	float fxnoise;			// fxnoise
	float z_f32;
};

struct TriggerSound {
	char path[];		// sound
	float soundramp;	// soundramp
	u8 z_u8;
	float volume;		// sound
};

struct Trigger {
	BitFlags flags;
	Transform transform;
	u32 type;					// type
	float sphere_size;			// size
	Vector box_size;			// size = 2.0 * this
	float polygon_size;			// size
	Vec<Vertex> polygon_vertices;
	TriggerSound sound;
};

struct ScriptSound {
	u32 kind;
	char name[];
};

struct ValueTransition {
	char variable[];
	u8 kind;
	float transition_time;
	double time;
	u8 z_u8_4[4];
};

using LuaTable;

enum LuaType : u32 {
	NIL = 0,
	Boolean = 1,
	Number = 3,
	String = 4,
	Table = 5,
	Reference = 0xFFFFFFFB
};

struct LuaTableEntry {
	LuaType key_type;
	if (key_type != LuaType::NIL) {
		if (key_type == LuaType::Boolean)
			bool key;
		else if (key_type == LuaType::Number)
			double key;
		else if (key_type == LuaType::String)
			char key[];
		else if (key_type == LuaType::Table)
			LuaTable key;
		else if (key_type == LuaType::Reference)
			u32 key;

		LuaType value_type;
		if (value_type == LuaType::Boolean)
			bool value;
		else if (value_type == LuaType::Number)
			double value;
		else if (value_type == LuaType::String)
			char value[];
		else if (value_type == LuaType::Table)
			LuaTable value;
		else if (value_type == LuaType::Reference)
			u32 value;
	}
};

struct LuaTable {
	LuaTableEntry entries[while (std::mem::read_unsigned($, 4) != LuaType::NIL)];
	u32 end;
};

struct Script {
	BitFlags flags;
	char file[];			// file
	Vec<Registry> params;	// param%d
	float last_update;
	float time;
	u8 z_u8_4[4];
	LuaTable table;
	Vec<u32> entity_handles;
	Vec<ScriptSound> sounds;
	Vec<ValueTransition> transitions;
};

using Entity;

struct Entity {
	EntityKind kind;
	u32 handle;
	SmallVec<Tag> tags;	// tags
	char desc[];		// desc
	if (kind == EntityKind::Body)
        Body body;
	else if (kind == EntityKind::Shape)
		Shape shape;
	else if (kind == EntityKind::Light)
		Light light;
	else if (kind == EntityKind::Location)
		Location location;
	else if (kind == EntityKind::Water)
		Water water;
	else if (kind == EntityKind::Joint)
		Joint joint;
	else if (kind == EntityKind::Vehicle)
		Vehicle vehicle;
	else if (kind == EntityKind::Wheel)
		Wheel wheel;
	else if (kind == EntityKind::Screen)
		Screen screen;
	else if (kind == EntityKind::Trigger)
		Trigger trigger;
	else if (kind == EntityKind::Script)
		Script script;
	else
		std::assert(true, "Invalid entity kind");
	Vec<Entity> children;
	u32 beef_beef;
	std::assert(beef_beef == 0xBEEFBEEF, "Invalid cow");
} [[single_color]];

struct Scene {
	char magic[5];
	u8 version[3] [[sealed, format("format_version")]];
	char level[];
	u32 driven;
	Vector shadowVolume;
	Transform spawnpoint;
	u32 z_u32_4[4] [[hidden]];
	PostProcessing postpro;
	Player player;
	Environment environment;
	Boundary boundary;
	Vec<Fire> fires;
	Vec<Palette> palettes;
	Vec<Registry> registry [[single_color]];
	/*Vec<Entity> entities;
	u32 z_u32;
	u8 z_u8_9[9];*/
} [[inline]];

fn format_version(ref auto version) {
	return std::format("{}.{}.{}", version[0], version[1], version[2]);
};

Scene scene @ 0x00;
