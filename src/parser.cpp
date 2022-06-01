#include <assert.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <experimental/filesystem>

#include "parser.h"
#include "vox_writer.h"
#include "zlib_utils.h"

using namespace std::experimental::filesystem;

TDBIN::TDBIN(const ConverterParams& params) {
	this->params = params;
	const char* input = params.bin_path.c_str();
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
		if (!IsFileCompressed(output)) {
			strcpy(filename, output);
		} else {
			printf("Error: Failed to uncompress file\n");
			exit(EXIT_FAILURE);
		}
		delete[] output;
	}

	printf("Parsing file...\n");
	bin_file = fopen(filename, "rb");
	if (bin_file == NULL) {
		printf("Error: Could not open %s for reading\n", filename);
		exit(EXIT_FAILURE);
	}
	delete[] filename;
}
TDBIN::~TDBIN() {
	fclose(bin_file);
	for (unsigned int i = 0; i < scene.entities.getSize(); i++)
		delete scene.entities[i];
	for (map<uint32_t, MV_FILE*>::iterator it = vox_files.begin(); it != vox_files.end(); it++)
		delete it->second;
	vox_files.clear();
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
		if (c != '\0')
			str += c;
	} while (c != '\0');
	return str;
}

Registry TDBIN::ReadRegistry() {
	Registry entry;
	entry.key = ReadString();
	entry.value = ReadString();
	return entry;
}

Rgba TDBIN::ReadRgba() {
	Rgba color;
	fread(&color, sizeof(Rgba), 1, bin_file);
	return color;
}

Transform TDBIN::ReadTransform() {
	Transform transform;
	fread(&transform, sizeof(Transform), 1, bin_file);
	return transform;
}

Fire TDBIN::ReadFire() {
	Fire fire;
	fire.entity_handle = ReadInt();
	for (int i = 0; i < 3; i++)
		fire.pos[i] = ReadFloat();
	fire.max_time = ReadFloat();
	fire.time = ReadFloat();
	for (int i = 0; i < 6; i++)
		fire.z_u8_6[i] = ReadByte();
	return fire;
}

Palette TDBIN::ReadPalette() {
	Palette p;
	for (int i = 0; i < 256; i++) {
		p.materials[i].kind = ReadByte();
		p.materials[i].rgba = ReadRgba();
		p.materials[i].reflectivity = ReadFloat();
		p.materials[i].shinyness = ReadFloat();
		p.materials[i].metalness = ReadFloat();
		p.materials[i].emissive = ReadFloat();
		p.materials[i].replacable = ReadByte() != 0;
	}
	fread(&p.tint_tables, sizeof(uint8_t), 2 * 4 * 256, bin_file);
	p.z_u8 = ReadByte();
	return p;
}

Rope TDBIN::ReadRope() {
	Rope rope;
	rope.color = ReadRgba();
	rope.z_f32 = ReadFloat();
	rope.strength = ReadFloat();
	rope.maxstretch = ReadFloat();
	rope.slack = ReadFloat();
	rope.z2_f32 = ReadFloat();
	rope.z_u8 = ReadByte();

	int knot_count = ReadInt();
	rope.knots.resize(knot_count);
	for (int i = 0; i < knot_count; i++) {
		for (int j = 0; j < 3; j++)
			rope.knots[i].from[j] = ReadFloat();
		for (int j = 0; j < 3; j++)
			rope.knots[i].to[j] = ReadFloat();
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
	properties.z2_f32 = ReadFloat();
	properties.z_u8 = ReadByte();
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
		assert(encoded_length % 2 == 0);
		voxels.palette_index.resize(volume);
		int k = 0;
		for (int i = 0; i < encoded_length / 2; i++) {
			int run_length = ReadByte();
			uint8_t voxel_index = ReadByte();
			for (int j = 0; j <= run_length; j++) {
				voxels.palette_index[k] = voxel_index;
				k++;
			}
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
			value.Table = new LuaTable;
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
	Entity* entity = new Entity;
	entity->kind_byte = ReadByte();
	entity->handle = ReadInt();

	entity_mapping[entity->handle] = entity;

	uint8_t tag_count = ReadByte();
	entity->tags.resize(tag_count);
	for (uint8_t i = 0; i < tag_count; i++)
		entity->tags[i] = ReadTag();

	entity->desc = ReadString();
	entity->kind = ReadEntityKind(entity->kind_byte);

	int childrens = ReadInt();
	//printf("%s with handle %d has %d childrens\n", EntityKindName[entity->kind_byte], entity->handle, childrens);
	entity->children.resize(childrens);
	for (int i = 0; i < childrens; i++) {
		entity->children[i] = ReadEntity();
		entity->children[i]->parent = entity;
	}

	entity->beef_beef = ReadInt();
	assert(entity->beef_beef == 0xBEEFBEEF);
	return entity;
}

Body* TDBIN::ReadBody() {
	Body* res = new Body;
	res->entity_flags = ReadWord();
	res->transform = ReadTransform();
	for (int i = 0; i < 3; i++)
		res->velocity[i] = ReadFloat();
	for (int i = 0; i < 3; i++)
		res->angular_velocity[i] = ReadFloat();
	res->dynamic = ReadByte() != 0;
	res->body_flags = ReadByte();
	return res;
}

Shape* TDBIN::ReadShape() {
	Shape* res = new Shape;
	res->flags = ReadWord();
	res->transform = ReadTransform();
	res->shape_flags = ReadByte();
	for (int i = 0; i < 3; i++)
		res->z_u8_3[i] = ReadByte();
	res->density = ReadFloat();
	res->strength = ReadFloat();
	res->texture_tile = ReadWord();
	res->blendtexture_tile = ReadWord();
	res->texture_weight = ReadFloat();
	res->blendtexture_weight = ReadFloat();
	for (int i = 0; i < 3; i++)
		res->starting_world_position[i] = ReadFloat();
	res->z_f32 = ReadFloat();
	res->z1_u8 = ReadByte();
	res->z2_u8 = ReadByte();
	res->voxels = ReadVoxels();
	res->palette = ReadInt();
	res->scale = ReadFloat();
	for (int i = 0; i < 2; i++)
		res->z_u32_2[i] = ReadInt();
	res->z3_u8 = ReadByte();
	return res;
}

Light* TDBIN::ReadLight() {
	Light* light = new Light;
	light->is_on = ReadByte() != 0;
	light->type = ReadByte();
	light->transform = ReadTransform();
	light->color = ReadRgba();
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
	light->capsule_size = ReadFloat();
	for (int i = 0; i < 13; i++)
		light->z_u8_13[i] = ReadByte();
	light->z_f32 = ReadFloat();
	light->sound.path = ReadString();
	light->sound.volume = ReadFloat();
	light->glare = ReadFloat();
	return light;
}

Location* TDBIN::ReadLocation() {
	Location* location = new Location;
	location->flags = ReadWord();
	location->transform = ReadTransform();
	return location;
}

Water* TDBIN::ReadWater() {
	Water* water = new Water;
	water->flags = ReadWord();
	water->transform = ReadTransform();
	water->depth = ReadFloat();
	water->wave = ReadFloat();
	water->ripple = ReadFloat();
	water->motion = ReadFloat();
	water->foam = ReadFloat();
	water->color = ReadRgba();
	int vertex_count = ReadInt();
	water->water_vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		water->water_vertices[i].pos[0] = ReadFloat();
		water->water_vertices[i].pos[1] = ReadFloat();
	}
	return water;
}

Joint* TDBIN::ReadJoint() {
	Joint* joint = new Joint;
	joint->type = ReadInt();
	for (int i = 0; i < 2; i++)
		joint->shape_handles[i] = ReadInt();
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++)
			joint->shape_positions[i][j] = ReadFloat();
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++)
			joint->shape_axes[i][j] = ReadFloat();
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
	joint->sound = ReadByte() != 0;
	joint->autodisable = ReadByte() != 0;
	for (int i = 0; i < 2; i++)
		joint->z_u32_2[i] = ReadInt();
	if (joint->type == _Rope)
		joint->rope = ReadRope();
	return joint;
}

Vehicle* TDBIN::ReadVehicle() {
	Vehicle* vehicle = new Vehicle;
	vehicle->flags = ReadWord();
	vehicle->body_handle = ReadInt();
	vehicle->transform = ReadTransform();
	for (int i = 0; i < 3; i++)
		vehicle->velocity[i] = ReadFloat();
	for (int i = 0; i < 3; i++)
		vehicle->angular_velocity[i] = ReadFloat();
	vehicle->z1_f32 = ReadFloat();

	int wheel_count = ReadInt();
	vehicle->wheel_handles.resize(wheel_count);
	for (int i = 0; i < wheel_count; i++)
		vehicle->wheel_handles[i] = ReadInt();

	vehicle->properties = ReadVehicleProperties();

	for (int i = 0; i < 3; i++)
		vehicle->camera[i] = ReadFloat();
	for (int i = 0; i < 3; i++)
		vehicle->player[i] = ReadFloat();
	for (int i = 0; i < 3; i++)
		vehicle->exit[i] = ReadFloat();
	for (int i = 0; i < 3; i++)
		vehicle->propeller[i] = ReadFloat();
	vehicle->difflock = ReadFloat();
	vehicle->z2_f32 = ReadFloat();
	vehicle->z_u32 = ReadInt();
	vehicle->z1_u8 = ReadByte();
	vehicle->z3_f32 = ReadFloat();

	int ref_count = ReadInt();
	vehicle->refs.resize(ref_count);
	for (int i = 0; i < ref_count; i++)
		vehicle->refs[i] = ReadInt();

	int exhaust_count = ReadInt();
	vehicle->exhausts.resize(exhaust_count);
	for (int i = 0; i < exhaust_count; i++) {
		vehicle->exhausts[i].transform = ReadTransform();
		vehicle->exhausts[i].z_f32 = ReadFloat();
	}

	int vital_count = ReadInt();
	vehicle->vitals.resize(vital_count);
	for (int i = 0; i < vital_count; i++) {
		vehicle->vitals[i].body_handle = ReadInt();
		for (int j = 0; j < 3; j++)
			vehicle->vitals[i].pos[j] = ReadFloat();
		vehicle->vitals[i].z_f32 = ReadFloat();
		vehicle->vitals[i].shape_index = ReadInt();
	}

	vehicle->z4_f32 = ReadFloat();
	vehicle->z2_u8 = ReadByte();
	vehicle->z5_f32 = ReadFloat();
	return vehicle;
}

Wheel* TDBIN::ReadWheel() {
	Wheel* wheel = new Wheel;
	wheel->flags = ReadWord();
	wheel->vehicle = ReadInt();
	wheel->vehicle_body = ReadInt();
	wheel->body = ReadInt();
	wheel->shape = ReadInt();
	wheel->transform = ReadTransform();
	for (int i = 0; i < 7; i++)
		wheel->z_f32_7[i] = ReadFloat();
	wheel->steer = ReadFloat();
	wheel->drive = ReadFloat();
	for (int i = 0; i < 2; i++)
		wheel->travel[i] = ReadFloat();
	for (int i = 0; i < 2; i++)
		wheel->z1_f32_2[i] = ReadFloat();
	wheel->angular_speed = ReadFloat();
	for (int i = 0; i < 2; i++)
		wheel->z2_f32_2[i] = ReadFloat();
	return wheel;
}

Screen* TDBIN::ReadScreen() {
	Screen* screen = new Screen;
	screen->flags = ReadWord();
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
	screen->z_f32 = ReadFloat();
	return screen;
}

Trigger* TDBIN::ReadTrigger() {
	Trigger* trigger = new Trigger;
	trigger->flags = ReadWord();
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
	Script* script = new Script;
	script->flags = ReadWord();
	script->file = ReadString();

	int entries = ReadInt();
	script->params.resize(entries);
	for (int i = 0; i < entries; i++)
		script->params[i] = ReadRegistry();

	script->last_update = ReadFloat();
	script->time = ReadFloat();
	for (int i = 0; i < 4; i++)
		script->z_u8_4[i] = ReadByte();

	script->table = ReadLuaTable();

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

	int transition_count = ReadInt();
	script->transitions.resize(transition_count);
	for (int i = 0; i < transition_count; i++) {
		script->transitions[i].variable = ReadString();
		script->transitions[i].kind = ReadByte();
		script->transitions[i].transition_time = ReadFloat();
		script->transitions[i].time = ReadDouble();
		for (int j = 0; j < 4; j++)
			script->transitions[i].z_u8_4[j] = ReadByte();
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
			printf("Error: Invalid entity kind: %d\n", kind_byte);
			exit(EXIT_FAILURE);
			return NULL;
	}
}

void TDBIN::ReadPlayer() {
	scene.player.transform = ReadTransform();
	scene.player.yaw = ReadFloat();
	scene.player.pitch = ReadFloat();
	for (int i = 0; i < 3; i++)
		scene.player.velocity[i] = ReadFloat();
	scene.player.health = ReadFloat();
	for (int i = 0; i < 4; i++)
		scene.player.z_f32_4[i] = ReadFloat();
}

void TDBIN::ReadEnvironment() {
	Skybox* skybox = &scene.environment.skybox;
	skybox->texture = ReadString();
	skybox->tint = ReadRgba();
	skybox->brightness = ReadFloat();
	skybox->rot = ReadFloat();
	for (int i = 0; i < 3; i++)
		skybox->sun.tint_brightness[i] = ReadFloat();
	skybox->sun.colorTint = ReadRgba();
	for (int i = 0; i < 3; i++)
		skybox->sun.dir[i] = ReadFloat();
	skybox->sun.brightness = ReadFloat();
	skybox->sun.spread = ReadFloat();
	skybox->sun.length = ReadFloat();
	skybox->sun.fogScale = ReadFloat();
	skybox->sun.glare = ReadFloat();
	skybox->z_u8 = ReadByte();
	skybox->constant = ReadRgba();
	skybox->ambient = ReadFloat();
	skybox->ambientexponent = ReadFloat();

	for (int i = 0; i < 2; i++)
		scene.environment.exposure[i] = ReadFloat();
	scene.environment.brightness = ReadFloat();

	Fog* fog = &scene.environment.fog;
	fog->color = ReadRgba();
	fog->start = ReadFloat();
	fog->distance = ReadFloat();
	fog->amount = ReadFloat();
	fog->exponent = ReadFloat();

	EnvWater* water = &scene.environment.water;
	water->wetness = ReadFloat();
	water->puddleamount = ReadFloat();
	water->puddlesize = ReadFloat();
	water->rain = ReadFloat();

	scene.environment.nightlight = ReadByte() != 0;
	scene.environment.ambience.path = ReadString();
	scene.environment.ambience.volume = ReadFloat();
	scene.environment.slippery = ReadFloat();
	scene.environment.fogscale = ReadFloat();

	Snow* snow = &scene.environment.snow;
	for (int i = 0; i < 3; i++)
		snow->dir[i] = ReadFloat();
	snow->spread = ReadFloat();
	snow->amount = ReadFloat();
	snow->speed = ReadFloat();
	snow->onground = ReadByte() != 0;

	for (int i = 0; i < 3; i++)
		scene.environment.wind[i] = ReadFloat();
	scene.environment.waterhurt = ReadFloat();
}

void TDBIN::parse() {
	for (int i = 0; i < 5; i++)
		scene.magic[i] = ReadByte();
	for (int i = 0; i < 3; i++)
		scene.version[i] = ReadByte();
	scene.level = ReadString();
	scene.driven_vehicle = ReadInt();
	for (int i = 0; i < 3; i++)
		scene.shadowVolume[i] = ReadFloat();
	scene.spawnpoint = ReadTransform();

	for (int i = 0; i < 3; i++)
		scene.z_u32_3[i] = ReadInt();
	scene.postpro.brightness = ReadFloat();
	scene.postpro.colorbalance = ReadRgba();
	scene.postpro.saturation = ReadFloat();
	scene.postpro.gamma = ReadFloat();
	scene.postpro.bloom = ReadFloat();

	ReadPlayer();
	ReadEnvironment();

	int vertex_count = ReadInt();
	scene.boundary.vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		scene.boundary.vertices[i].pos[0] = ReadFloat();
		scene.boundary.vertices[i].pos[1] = ReadFloat();
	}
	scene.boundary.padleft = ReadFloat();
	scene.boundary.padtop = ReadFloat();
	scene.boundary.padright = ReadFloat();
	scene.boundary.padbottom = ReadFloat();

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

	int entity_count = ReadInt();
	scene.entities.resize(entity_count);
	for (int i = 0; i < entity_count; i++) {
		scene.entities[i] = ReadEntity();
		scene.entities[i]->parent = NULL;
	}
	printf("File parsed successfully!\n");
}

void ParseFile(ConverterParams params) {
	if (!exists(params.map_folder)) {
		create_directories(params.map_folder);
		create_directories(params.map_folder + "vox");
		create_directories(params.map_folder + "compounds");
	}

	TDBIN parser(params);
	parser.parse();
	parser.WriteScene();
	parser.WriteSpawnpoint();
	parser.WriteEnvironment();
	parser.WriteBoundary();
	parser.WritePostProcessing();
	parser.WriteEntities();
	parser.SaveXML();
	if (!params.xml_only)
		parser.SaveVoxFiles();
	printf("Map successfully converted!\n");
	progress = 1;
}
