#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <filesystem>

#include "parser.h"
#include "write_scene.h"
#include "zlib_utils.h"

using namespace std::filesystem;

void TDBIN::init(const char* input) {
	char* filename = new char[strlen(input) + 3];
	strcpy(filename, input);
	if (IsFileCompressed(input)) {
		printf("Unzipping file...\n");
		char* output = new char[strlen(input) + 3];
		strcpy(output, input);
		char* pos = strstr(output, ".bin");
		if (pos != NULL)
			*pos = '\0';
		strcat(output, ".tdbin");
		UncompressFile(input, output);
		strcpy(filename, output);
		delete[] output;
	}

	printf("Parsing file...\n");
	bin_file = fopen(filename, "rb");
	if (bin_file == NULL) {
		printf("[ERROR] Could not open %s for reading\n", filename);
		exit(EXIT_FAILURE);
	}
	delete[] filename;
}

TDBIN::~TDBIN() {
	fclose(bin_file);
	for (unsigned int i = 0; i < scene.entities.getSize(); i++)
		delete scene.entities[i];
}

uint8_t TDBIN::ReadByte() {
	uint8_t byte = 0;
	fread(&byte, sizeof(uint8_t), 1, bin_file);
	return byte;
}

uint16_t TDBIN::ReadWord() {
	uint16_t word = 0;
	fread(&word, sizeof(uint16_t), 1, bin_file);
	return word;
}

uint16_t TDBIN::ReadEntityFlags() {
	if (tdbin_version >= VERSION_0_9_2)
		return ReadWord();
	else
		return ReadByte();
}

uint32_t TDBIN::ReadInt() {
	uint32_t val = 0;
	fread(&val, sizeof(uint32_t), 1, bin_file);
	return val;
}

float TDBIN::ReadFloat() {
	float val = 0;
	fread(&val, sizeof(float), 1, bin_file);
	return val;
}

double TDBIN::ReadDouble() {
	double val = 0;
	fread(&val, sizeof(double), 1, bin_file);
	return val;
}

string TDBIN::ReadString() {
	string str;
	char c;
	do {
		fread(&c, sizeof(char), 1, bin_file);
		if (c != '\0') str += c;
	} while (c != '\0');
	return str;
}

Registry TDBIN::ReadRegistry() {
	Registry entry;
	entry.key = ReadString();
	entry.value = ReadString();
	return entry;
}

Color TDBIN::ReadColor() {
	Color color;
	fread(&color, sizeof(Color), 1, bin_file);
	return color;
}

Vector TDBIN::ReadVector() {
	Vector vec;
	fread(&vec, sizeof(Vector), 1, bin_file);
	return vec;
}

Transform TDBIN::ReadTransform() {
	Transform transform;
	fread(&transform, sizeof(Transform), 1, bin_file);
	return transform;
}

Fire TDBIN::ReadFire() {
	Fire fire;
	fire.shape = ReadInt();
	fire.position = ReadVector();
	fire.max_time = ReadFloat();
	fire.time = ReadFloat();
	fire.painted = ReadByte() != 0;
	fire.broken = ReadByte() != 0;
	fire.spawned_count = ReadInt();
	return fire;
}

Palette TDBIN::ReadPalette() {
	Palette p;
	for (int i = 0; i < 256; i++) {
		p.materials[i].kind = ReadByte();
		if (p.materials[i].kind > (uint8_t)MaterialKind::Unphysical) {
			printf("[ERROR] Invalid Material %d at index %d\n", p.materials[i].kind, i);
			exit(EXIT_FAILURE);
		}
		p.materials[i].rgba = ReadColor();
		p.materials[i].reflectivity = ReadFloat();
		p.materials[i].shinyness = ReadFloat();
		p.materials[i].metalness = ReadFloat();
		p.materials[i].emissive = ReadFloat();
		p.materials[i].replaceable = ReadByte() != 0;
	}
	p.z_u8 = ReadByte();
	fread(&p.black_tint, sizeof(uint8_t), 4 * 256, bin_file);
	fread(&p.yellow_tint, sizeof(uint8_t), 4 * 256, bin_file);
	if (tdbin_version >= VERSION_1_5_0)
		fread(&p.other_tint, sizeof(uint8_t), 4 * 256, bin_file);
	return p;
}

Rope TDBIN::ReadRope() {
	Rope rope;
	rope.color = ReadColor();
	rope.z_f32 = ReadFloat();
	rope.strength = ReadFloat();
	rope.maxstretch = ReadFloat();
	rope.slack = ReadFloat();
	rope.z2_f32 = ReadFloat();
	rope.z_u8 = ReadByte();

	int knot_count = ReadInt();
	rope.knots.resize(knot_count);
	for (int i = 0; i < knot_count; i++) {
		rope.knots[i].from = ReadVector();
		rope.knots[i].to = ReadVector();
	}
	return rope;
}

Tag TDBIN::ReadTag() {
	Tag tag;
	tag.name = ReadString();
	tag.value = ReadString();
	return tag;
}

VehicleProperties TDBIN::ReadVehicleProperties() {
	VehicleProperties properties;
	properties.topspeed = ReadFloat();
	properties.z1_f32 = ReadFloat();
	properties.spring = ReadFloat();
	properties.damping = ReadFloat();
	properties.acceleration = ReadFloat();
	properties.strength = ReadFloat();
	properties.friction = ReadFloat();
	properties.max_steering_angle = ReadFloat();
	properties.handbrake = ReadByte() != 0;
	properties.antispin = ReadFloat();
	properties.steerassist = ReadFloat();
	properties.z3_f32 = ReadFloat();
	properties.antiroll = ReadFloat();
	properties.sound.name = ReadString();
	properties.sound.pitch = ReadFloat();
	return properties;
}

Voxels TDBIN::ReadVoxels() {
	Voxels voxels;
	for (int i = 0; i < 3; i++)
		voxels.size[i] = ReadInt();

	int volume = voxels.size[0] * voxels.size[1] * voxels.size[2];
	if (volume > 0) {
		int encoded_length = ReadInt();
		voxels.palette_indexes.resize(encoded_length / 2);
		for (int i = 0; i < encoded_length / 2; i++) {
			uint8_t run_length = ReadByte();
			uint8_t voxel_index = ReadByte();
			voxels.palette_indexes[i] = { run_length, voxel_index };
		}
	}
	return voxels;
}

LuaValue TDBIN::ReadLuaValue(uint32_t key_type) {
	LuaValue value;
	switch (key_type) {
		case Boolean:
			value.Boolean = ReadByte() != 0;
			break;
		case Number:
			value.Number = ReadDouble();
			break;
		case String: {
			string text = ReadString();
			value.String = new char[text.length() + 1];
			strcpy(value.String, text.c_str());
		}
			break;
		case Table: {
			LuaTable table = ReadLuaTable();
			value.Table = new LuaTable();
			for (unsigned int i = 0; i < table.size(); i++)
				value.Table->push_back(table[i]);
		}
			break;
		case Reference:
			value.Reference = ReadInt();
			break;
	}
	return value;
}

LuaTable TDBIN::ReadLuaTable() {
	LuaTable table;
	do {
		LuaTableEntry table_entry;
		table_entry.key_type = ReadInt();
		if (table_entry.key_type == NIL) break;
		table_entry.key = ReadLuaValue(table_entry.key_type);
		table_entry.value_type = ReadInt();
		table_entry.value = ReadLuaValue(table_entry.value_type);
		table.push_back(table_entry);
	} while (true);
	return table;
}

Entity* TDBIN::ReadEntity() {
	Entity* entity = new Entity();
	entity->kind_byte = ReadByte();

	entity->handle = ReadInt();
	entity_mapping[entity->handle] = entity;
	//printf("Reading %s with handle %d\n", EntityKindName[entity->kind_byte], entity->handle);

	uint8_t tag_count = ReadByte();
	entity->tags.resize(tag_count);
	for (uint8_t i = 0; i < tag_count; i++)
		entity->tags[i] = ReadTag();

	entity->desc = ReadString();
	if (entity->kind_byte != KindLight && entity->kind_byte != KindJoint) // Ah, yes... consistency
		entity->flags = ReadEntityFlags();
	entity->kind = ReadEntityKind(entity->kind_byte);

	int childrens = ReadInt();
	entity->children.resize(childrens);
	for (int i = 0; i < childrens; i++) {
		entity->children[i] = ReadEntity();
		entity->children[i]->parent = entity;
	}

	entity->beef_beef = ReadInt();
	if (entity->beef_beef != 0xBEEFBEEF) {
		printf("[ERROR] Invalid Cow: %08X\n", entity->beef_beef);
		exit(EXIT_FAILURE);
	}
	return entity;
}

Body* TDBIN::ReadBody() {
	Body* body = new Body();
	body->transform = ReadTransform();
	body->velocity = ReadVector();
	body->angular_velocity = ReadVector();
	body->dynamic = ReadByte() != 0;
	if (tdbin_version < VERSION_0_9_0)
		ReadByte();
	body->active = ReadByte();
	if (tdbin_version >= VERSION_1_5_4) {
		body->friction = ReadFloat();
		body->friction_mode = ReadByte();
		body->restitution = ReadFloat();
		body->restitution_mode = ReadByte();
	}
	return body;
}

Shape* TDBIN::ReadShape() {
	Shape* shape = new Shape();
	shape->transform = ReadTransform();
	shape->shape_flags = ReadWord();
	shape->collision_layer = ReadByte();
	shape->collision_mask = ReadByte();
	shape->density = ReadFloat();
	shape->strength = ReadFloat();

	if (tdbin_version >= VERSION_0_9_0) {
		shape->texture_tile = ReadWord();
		shape->blendtexture_tile = ReadWord();
		shape->texture_weight = ReadFloat();
		shape->blendtexture_weight = ReadFloat();
		shape->texture_offset = ReadVector();
	} else {
		shape->texture_tile = ReadInt();
		shape->texture_offset = ReadVector();
		shape->texture_weight = ReadFloat();
	}

	shape->emissive_scale = ReadFloat();
	shape->is_broken = ReadByte() != 0;
	shape->shape_type = ReadByte();
	shape->voxels = ReadVoxels();
	shape->palette = ReadInt();
	shape->scale = ReadFloat();

	for (int i = 0; i < 2; i++)
		shape->z_u32_2[i] = ReadInt();
	if (tdbin_version >= VERSION_0_7_0)
		shape->is_disconnected = ReadByte();
	if (tdbin_version >= VERSION_1_5_4)
		shape->z3_u8 = ReadByte();
	return shape;
}

Light* TDBIN::ReadLight() {
	Light* light = new Light();
	light->is_on = ReadByte() != 0;
	light->type = ReadByte();
	if (tdbin_version < VERSION_0_7_0 && light->type > Sphere)
		light->type++; // Skip Capsule

	light->transform = ReadTransform();
	light->color = ReadColor();
	if (tdbin_version >= VERSION_0_5_0)
		light->scale = ReadFloat();
	light->reach = ReadFloat();
	light->size = ReadFloat();
	light->unshadowed = ReadFloat();
	light->angle = ReadFloat();
	light->penumbra = ReadFloat();
	light->fogiter = ReadFloat();
	light->fogscale = ReadFloat();
	for (int i = 0; i < 2; i++)
		light->area_size[i] = ReadFloat();
	if (tdbin_version >= VERSION_0_7_0)
		light->capsule_size = ReadFloat();
	light->position = ReadVector();
	light->index = ReadByte();
	light->z_f32 = ReadFloat();
	light->sound.path = ReadString();
	light->sound.volume = ReadFloat();
	light->glare = ReadFloat();
	return light;
}

Location* TDBIN::ReadLocation() {
	Location* location = new Location();
	location->transform = ReadTransform();
	return location;
}

Water* TDBIN::ReadWater() {
	Water* water = new Water();
	water->transform = ReadTransform();
	water->depth = ReadFloat();
	water->wave = ReadFloat();
	water->ripple = ReadFloat();
	water->motion = ReadFloat();
	water->foam = ReadFloat();
	if (tdbin_version >= VERSION_0_9_0)
		water->color = ReadColor();
	if (tdbin_version >= VERSION_1_4_0)
		water->visibility = ReadFloat();
	int vertex_count = ReadInt();
	water->water_vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		water->water_vertices[i].pos[0] = ReadFloat();
		water->water_vertices[i].pos[1] = ReadFloat();
	}
	return water;
}

Joint* TDBIN::ReadJoint() {
	Joint* joint = new Joint();
	joint->type = ReadInt();
	for (int i = 0; i < 2; i++)
		joint->shape_handles[i] = ReadInt();
	for (int i = 0; i < 2; i++)
			joint->shape_positions[i] = ReadVector();
	for (int i = 0; i < 2; i++)
		joint->shape_axes[i] = ReadVector();
	joint->connected = ReadByte() != 0;
	joint->collide = ReadByte() != 0;
	joint->rotstrength = ReadFloat();
	joint->rotspring = ReadFloat();
	for (int i = 0; i < 4; i++)
		joint->ball_rot[i] = ReadFloat();
	for (int i = 0; i < 2; i++)
		joint->limits[i] = ReadFloat();
	for (int i = 0; i < 2; i++)
		joint->z_f32_2[i] = ReadFloat();
	joint->size = ReadFloat();
	if (tdbin_version >= VERSION_0_9_0)
		joint->sound = ReadByte() != 0;
	if (tdbin_version >= VERSION_0_9_5)
		joint->autodisable = ReadByte() != 0;
	if (tdbin_version >= VERSION_0_9_0)
		for (int i = 0; i < 2; i++)
			joint->z_f32_4[i] = ReadFloat();
	if (joint->type == _Rope)
		joint->rope = ReadRope();
	return joint;
}

Vehicle* TDBIN::ReadVehicle() {
	Vehicle* vehicle = new Vehicle();
	vehicle->body_handle = ReadInt();
	vehicle->transform = ReadTransform();
	vehicle->transform2 = ReadTransform();

	int wheel_count = ReadInt();
	vehicle->wheel_handles.resize(wheel_count);
	for (int i = 0; i < wheel_count; i++)
		vehicle->wheel_handles[i] = ReadInt();

	vehicle->properties = ReadVehicleProperties();

	vehicle->camera = ReadVector();
	vehicle->player = ReadVector();
	vehicle->exit = ReadVector();
	vehicle->propeller = ReadVector();
	vehicle->difflock = ReadFloat();
	vehicle->health = ReadFloat();
	vehicle->main_voxel_count = ReadInt();
	if (tdbin_version != VERSION_0_3_0) {
		vehicle->z1_u8 = ReadByte();
		vehicle->drivable = ReadFloat();
	}

	int ref_count = ReadInt();
	vehicle->refs.resize(ref_count);
	for (int i = 0; i < ref_count; i++)
		vehicle->refs[i] = ReadInt();

	int exhaust_count = ReadInt();
	vehicle->exhausts.resize(exhaust_count);
	for (int i = 0; i < exhaust_count; i++) {
		vehicle->exhausts[i].transform = ReadTransform();
		if (tdbin_version >= VERSION_0_7_0)
			vehicle->exhausts[i].strength = ReadFloat();
	}

	int vital_count = ReadInt();
	vehicle->vitals.resize(vital_count);
	for (int i = 0; i < vital_count; i++) {
		vehicle->vitals[i].body_handle = ReadInt();
		vehicle->vitals[i].pos = ReadVector();
		vehicle->vitals[i].z_f32 = ReadFloat();
		vehicle->vitals[i].shape_handle = ReadInt();
	}
	vehicle->z2_f32 = ReadFloat();
	if (tdbin_version >= VERSION_0_9_0) {
		vehicle->z2_u8 = ReadByte();
		vehicle->brokenthreshold = ReadFloat();
	}
	if (tdbin_version >= VERSION_1_5_4)
		vehicle->z3_f32 = ReadFloat();
	return vehicle;
}

Wheel* TDBIN::ReadWheel() {
	Wheel* wheel = new Wheel();
	wheel->vehicle = ReadInt();
	wheel->vehicle_body = ReadInt();
	wheel->body = ReadInt();
	wheel->shape = ReadInt();
	if (tdbin_version >= VERSION_1_3_0) {
		wheel->z1_f32_handle = ReadInt();
		for (int i = 0; i < 3; i++)
			wheel->z1_f32_3[i] = ReadInt();
		wheel->z_u8 = ReadByte();
	}
	wheel->transform = ReadTransform();
	wheel->empty_transform = ReadTransform();
	wheel->steer = ReadFloat();
	wheel->drive = ReadFloat();
	for (int i = 0; i < 2; i++)
		wheel->travel[i] = ReadFloat();
	wheel->radius = ReadFloat();
	wheel->width = ReadFloat();
	wheel->angular_speed = ReadFloat();
	for (int i = 0; i < 2; i++)
		wheel->z2_f32_2[i] = ReadFloat();
	return wheel;
}

Screen* TDBIN::ReadScreen() {
	Screen* screen = new Screen();
	screen->transform = ReadTransform();
	for (int i = 0; i < 2; i++)
		screen->size[i] = ReadFloat();
	screen->bulge = ReadFloat();
	for (int i = 0; i < 2; i++)
		screen->resolution[i] = ReadInt();
	screen->script = ReadString();
	screen->enabled = ReadByte() != 0;
	screen->interactive = ReadByte() != 0;
	screen->emissive = ReadFloat();
	screen->fxraster = ReadFloat();
	screen->fxca = ReadFloat();
	screen->fxnoise = ReadFloat();
	screen->fxglitch = ReadFloat();
	return screen;
}

Trigger* TDBIN::ReadTrigger() {
	Trigger* trigger = new Trigger();
	trigger->transform = ReadTransform();
	trigger->type = ReadInt();
	trigger->sphere_size = ReadFloat();
	for (int i = 0; i < 3; i++)
		trigger->box_size[i] = ReadFloat();
	trigger->polygon_size = ReadFloat();
	int vertex_count = ReadInt();
	trigger->polygon_vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		trigger->polygon_vertices[i].pos[0] = ReadFloat();
		trigger->polygon_vertices[i].pos[1] = ReadFloat();
	}
	trigger->sound.path = ReadString();
	trigger->sound.soundramp = ReadFloat();
	trigger->sound.z_u8 = ReadByte();
	trigger->sound.volume = ReadFloat();
	return trigger;
};

Script* TDBIN::ReadScript() {
	Script* script = new Script();
	script->file = ReadString();

	int entries = ReadInt();
	script->params.resize(entries);
	for (int i = 0; i < entries; i++)
		script->params[i] = ReadRegistry();

	script->last_update = ReadFloat();
	script->time = ReadFloat();
	script->z_u32 = ReadInt();

	script->table = ReadLuaTable();

	if (tdbin_version < VERSION_0_5_0) {
		int unk_count = ReadInt();
		for (int i = 0; i < unk_count; i++)
			ReadInt();
	}

	int entities = ReadInt();
	script->entity_handles.resize(entities);
	for (int i = 0; i < entities; i++)
		script->entity_handles[i] = ReadInt();

	int sound_count = ReadInt();
	script->sounds.resize(sound_count);
	for (int i = 0; i < sound_count; i++) {
		script->sounds[i].kind = ReadInt();
		script->sounds[i].name = ReadString();
	}

	if (tdbin_version >= VERSION_0_7_4) {
		int transition_count = ReadInt();
		script->transitions.resize(transition_count);
		for (int i = 0; i < transition_count; i++) {
			script->transitions[i].variable = ReadString();
			script->transitions[i].kind = ReadByte();
			script->transitions[i].transition_time = ReadFloat();
			script->transitions[i].time = ReadFloat();
			script->transitions[i].z1_f32 = ReadFloat();
			script->transitions[i].z2_f32 = ReadFloat();
		}
	}
	return script;
}

void* TDBIN::ReadEntityKind(uint8_t kind_byte) {
	switch (kind_byte) {
		case KindBody:
			return ReadBody();
		case KindShape:
			return ReadShape();
		case KindLight:
			return ReadLight();
		case KindLocation:
			return ReadLocation();
		case KindWater:
			return ReadWater();
		case KindJoint:
			return ReadJoint();
		case KindVehicle:
			return ReadVehicle();
		case KindWheel:
			return ReadWheel();
		case KindScreen:
			return ReadScreen();
		case KindTrigger:
			return ReadTrigger();
		case KindScript:
			return ReadScript();
		default:
			printf("[ERROR] Invalid entity kind: %d\n", kind_byte);
			exit(EXIT_FAILURE);
			return NULL;
	}
}

void TDBIN::ReadPlayer() {
	Player* player = &scene.player;
	player->transform = ReadTransform();
	player->pitch = ReadFloat();
	player->yaw = ReadFloat();
	player->velocity = ReadVector();
	player->health = ReadFloat();

	player->transition_timer = ReadFloat();
	player->time_underwater = ReadFloat();
	if (tdbin_version >= VERSION_0_9_0) {
		player->bluetide_timer = ReadFloat();
		player->bluetide_power = ReadFloat();
	}
}

void TDBIN::ReadEnvironment() {
	Environment* environment = &scene.environment;
	Skybox* skybox = &environment->skybox;

	skybox->texture = ReadString();
	skybox->tint = ReadColor();
	if (tdbin_version >= VERSION_0_8_0)
		skybox->brightness = ReadFloat();
	skybox->rot = ReadFloat();
	skybox->sun.tint_brightness = ReadVector();
	skybox->sun.colortint = ReadColor();
	skybox->sun.dir = ReadVector();
	skybox->sun.brightness = ReadFloat();
	if (tdbin_version == VERSION_0_3_0)
		ReadByte();
	skybox->sun.spread = ReadFloat();
	skybox->sun.length = ReadFloat();
	skybox->sun.fogscale = ReadFloat();
	skybox->sun.glare = ReadFloat();
	if (tdbin_version != VERSION_0_3_0)
		skybox->auto_sun_dir = ReadByte() != 0;
	if (tdbin_version >= VERSION_0_7_0)
		skybox->constant = ReadColor();
	skybox->ambient = ReadFloat();
	if (tdbin_version >= VERSION_0_7_0)
		skybox->ambientexponent = ReadFloat();

	for (int i = 0; i < 2; i++)
		environment->exposure[i] = ReadFloat();
	environment->brightness = ReadFloat();

	Fog* fog = &environment->fog;
	if (tdbin_version != VERSION_0_3_0)
		fog->color = ReadColor();
	fog->start = ReadFloat();
	fog->distance = ReadFloat();
	fog->amount = ReadFloat();
	fog->exponent = ReadFloat();

	EnvWater* water = &environment->water;
	water->wetness = ReadFloat();
	water->puddleamount = ReadFloat();
	water->puddlesize = ReadFloat();
	water->rain = ReadFloat();

	environment->nightlight = ReadByte() != 0;
	environment->ambience.path = ReadString();
	environment->ambience.volume = ReadFloat();
	if (tdbin_version >= VERSION_0_5_5)
		environment->slippery = ReadFloat();
	if (tdbin_version >= VERSION_0_7_0)
		environment->fogscale = ReadFloat();

	if (tdbin_version >= VERSION_0_9_0) {
		Snow* snow = &environment->snow;
		snow->dir = ReadVector();
		snow->spread = ReadFloat();
		snow->amount = ReadFloat();
		snow->speed = ReadFloat();
		snow->onground = ReadByte() != 0;

		environment->wind = ReadVector();
		environment->waterhurt = ReadFloat();
	} else if (tdbin_version >= VERSION_0_7_1) {
		// 0.7.1 -> 0.8.0
		for (int i = 0; i < 8; i++)
			ReadFloat();
		ReadByte();
	}
}

void TDBIN::parse() {
	for (int i = 0; i < 4; i++) // Read only 4 bytes
		scene.magic[i] = ReadByte();

	if (scene.magic[0] != 'T' || scene.magic[1] != 'D' || scene.magic[2] != 'B' || scene.magic[3] != 'I')
		tdbin_version = VERSION_0_3_0;

	if (tdbin_version != VERSION_0_3_0) {
		scene.magic[4] = ReadByte();
		if (scene.magic[4] != 'N') {
			printf("[ERROR] File is corrupted or may not be a Teardown map.\n");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < 3; i++)
			scene.version[i] = ReadByte();
		tdbin_version = scene.version[0] * 100 + scene.version[1] * 10 + scene.version[2];

		if (tdbin_version >= VERSION_0_5_1)
			scene.level_id = ReadString();
		if (tdbin_version >= VERSION_1_5_0) {
			scene.level_path = ReadString();
			scene.layers = ReadString();
			scene.mod = ReadString();
			scene.aaa1 = ReadInt();

			int entries = ReadInt();
			scene.enabled_mods.resize(entries);
			for (int i = 0; i < entries; i++)
				scene.enabled_mods[i] = ReadRegistry();
		}
		if (tdbin_version >= VERSION_1_5_4) {
			int entries = ReadInt();
			scene.spawned_mods.resize(entries);
			for (int i = 0; i < entries; i++)
				scene.spawned_mods[i] = ReadRegistry();
		}
		scene.driven_vehicle = ReadInt();
	}

	scene.shadow_volume = ReadVector();
	scene.spawnpoint = ReadTransform();
	if (tdbin_version >= VERSION_0_5_1) {
		scene.world_body = ReadInt();
		scene.flashlight = ReadInt();
	}
	if (tdbin_version >= VERSION_0_7_0)
		scene.explosion_lua = ReadInt();
	if (tdbin_version >= VERSION_1_1_0)
		scene.achievements_lua = ReadInt();

	if (tdbin_version >= VERSION_0_7_0) {
		PostProcessing* postpro = &scene.postpro;
		postpro->brightness = ReadFloat();
		postpro->colorbalance = ReadColor();
		postpro->saturation = ReadFloat();
		postpro->gamma = ReadFloat();
		if (tdbin_version >= VERSION_0_8_0)
			postpro->bloom = ReadFloat();
	}

	ReadPlayer();
	ReadEnvironment();

	Boundary* boundary = &scene.boundary;
	int vertex_count = ReadInt();
	boundary->vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		boundary->vertices[i].pos[0] = ReadFloat();
		boundary->vertices[i].pos[1] = ReadFloat();
	}
	if (tdbin_version >= VERSION_0_9_0) {
		boundary->padleft = ReadFloat();
		boundary->padtop = ReadFloat();
		boundary->padright = ReadFloat();
		boundary->padbottom = ReadFloat();
		if (tdbin_version >= VERSION_1_5_4)
			boundary->maxheight = ReadFloat();
	}

	int fire_count = ReadInt();
	scene.fires.resize(fire_count);
	for (int i = 0; i < fire_count; i++)
		scene.fires[i] = ReadFire();

	int palette_count = ReadInt();
	scene.palettes.resize(palette_count);
	for (int i = 0; i < palette_count; i++)
		scene.palettes[i] = ReadPalette();

	int entries = ReadInt();
	scene.registry.resize(entries);
	for (int i = 0; i < entries; i++)
		scene.registry[i] = ReadRegistry();

	int top_entity_count = ReadInt();
	scene.entities.resize(top_entity_count);
	for (int i = 0; i < top_entity_count; i++) {
		scene.entities[i] = ReadEntity();
		scene.entities[i]->parent = NULL;
	}
	/*if (fgetc(bin_file) != EOF)
		throw runtime_error("File size mismatch.");*/
	printf("File parsed successfully!\n");
}

void ParseFile(ConverterParams params) {
	WriteXML parser(params);
	try {
		parser.parse();
	} catch(const std::bad_alloc& e) {
		printf("You're a few terabytes low on RAM or this application crashed.\n");
		printf("It's most likely the second.\n");
		exit(EXIT_FAILURE);
	}
	if (!exists(params.map_folder)) {
		create_directories(params.map_folder);
		create_directories(params.map_folder + (params.legacy_format ? "custom" : "vox"));
	}
	parser.WriteScene();
	parser.WriteSpawnpoint();
	parser.WriteEnvironment();
	parser.WriteBoundary();
	parser.WritePostProcessing();
	parser.WriteEntities();
	parser.SaveXML();
	parser.SaveVoxFiles();
	printf("Map successfully converted!\n");
	progress = 1;
}
