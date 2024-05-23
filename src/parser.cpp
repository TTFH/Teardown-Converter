#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <filesystem>

#include "parser.h"
#include "write_scene.h"
#include "zlib_utils.h"

using namespace std::filesystem;

void TDBIN::init(const char* input) {
	int len = strlen(input) + 3; // for 'td'
	char* filename = new char[len];
	strcpy(filename, input);
	if (IsFileCompressed(input)) {
		printf("Unzipping file...\n");
		char* output = new char[len];
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

bool TDBIN::ReadBool() {
	return ReadByte() != 0;
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
	fire.painted = ReadBool();
	fire.broken = ReadBool();
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
		p.materials[i].is_tint = ReadBool();
	}
	p.z_u8 = ReadBool();
	fread(&p.black_tint, sizeof(uint8_t), 4 * 256, bin_file);
	fread(&p.yellow_tint, sizeof(uint8_t), 4 * 256, bin_file);
	fread(&p.rgba_tint, sizeof(uint8_t), 4 * 256, bin_file);
	return p;
}

Rope* TDBIN::ReadRope() {
	Rope* rope = new Rope();
	rope->color = ReadColor();
	rope->z_f32 = ReadFloat();
	rope->strength = ReadFloat();
	rope->maxstretch = ReadFloat();
	rope->slack = ReadFloat();
	rope->segment_length = ReadFloat();
	rope->active = ReadByte();

	int segments = ReadInt();
	rope->segments.resize(segments);
	for (int i = 0; i < segments; i++) {
		rope->segments[i].from = ReadVector();
		rope->segments[i].to = ReadVector();
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
	properties.top_speed_clamp = ReadFloat();
	properties.spring = ReadFloat();
	properties.damping = ReadFloat();
	properties.acceleration = ReadFloat();
	properties.strength = ReadFloat();
	properties.friction = ReadFloat();
	properties.max_steering_angle = ReadFloat();
	properties.handbrake = ReadBool();
	properties.antispin = ReadFloat();
	properties.steerassist = ReadFloat();
	properties.z_f32 = ReadFloat();
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
			value.Boolean = ReadBool();
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
	entity->type = ReadByte();

	entity->handle = ReadInt();
	entity_mapping[entity->handle] = entity;
	//printf("Reading %s with handle %d\n", EntityKindName[entity->type], entity->handle);

	uint8_t tag_count = ReadByte();
	entity->tags.resize(tag_count);
	for (uint8_t i = 0; i < tag_count; i++)
		entity->tags[i] = ReadTag();

	entity->desc = ReadString();
	if (entity->type != KindLight && entity->type != KindJoint) // Ah, yes... consistency
		entity->flags = ReadWord();
	entity->self = ReadEntityKind(entity->type);

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
	entity->parent = NULL;
	return entity;
}

Body* TDBIN::ReadBody() {
	Body* body = new Body();
	body->transform = ReadTransform();
	body->velocity = ReadVector();
	body->angular_velocity = ReadVector();
	body->dynamic = ReadBool();
	body->active = ReadByte();
	body->friction = ReadFloat();
	body->friction_mode = ReadByte();
	body->restitution = ReadFloat();
	body->restitution_mode = ReadByte();
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

	shape->texture_tile = ReadWord();
	shape->blendtexture_tile = ReadWord();
	shape->texture_weight = ReadFloat();
	shape->blendtexture_weight = ReadFloat();
	shape->texture_offset = ReadVector();

	shape->emissive_scale = ReadFloat();
	shape->is_broken = ReadBool();
	shape->has_voxels = ReadByte();
	shape->voxels = ReadVoxels();
	shape->palette = ReadInt();
	shape->scale = ReadFloat();

	for (int i = 0; i < 2; i++)
		shape->z_u32_2[i] = ReadInt();
	shape->is_disconnected = ReadByte();
	shape->z3_u8 = ReadByte();
	return shape;
}

Light* TDBIN::ReadLight() {
	Light* light = new Light();
	light->is_on = ReadBool();
	light->type = ReadByte();

	light->transform = ReadTransform();
	light->color = ReadColor();
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
	water->color = ReadColor();
	water->visibility = ReadFloat();
	int vertex_count = ReadInt();
	water->water_vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		water->water_vertices[i].x = ReadFloat();
		water->water_vertices[i].y = ReadFloat();
	}
	return water;
}

Joint* TDBIN::ReadJoint() {
	Joint* joint = new Joint();
	joint->type = ReadInt();
	for (int i = 0; i < 2; i++)
		joint->shapes[i] = ReadInt();
	for (int i = 0; i < 2; i++)
			joint->positions[i] = ReadVector();
	for (int i = 0; i < 2; i++)
		joint->axis[i] = ReadVector();
	joint->connected = ReadBool();
	joint->collide = ReadBool();
	joint->rotstrength = ReadFloat();
	joint->rotspring = ReadFloat();
	joint->hinge_rot.x = ReadFloat();
	joint->hinge_rot.y = ReadFloat();
	joint->hinge_rot.z = ReadFloat();
	joint->hinge_rot.w = ReadFloat();
	for (int i = 0; i < 2; i++)
		joint->limits[i] = ReadFloat();
	for (int i = 0; i < 2; i++)
		joint->z_f32_2[i] = ReadFloat();
	joint->size = ReadFloat();
	joint->sound = ReadBool();
	joint->autodisable = ReadBool();
	for (int i = 0; i < 2; i++)
		joint->z_f32_2[i] = ReadFloat();
	if (joint->type == _Rope)
		joint->rope = ReadRope();
	else
		joint->rope = NULL;
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
	vehicle->braking = ReadBool();
	vehicle->z1_f32 = ReadFloat();

	int ref_count = ReadInt();
	vehicle->refs.resize(ref_count);
	for (int i = 0; i < ref_count; i++)
		vehicle->refs[i] = ReadInt();

	int exhaust_count = ReadInt();
	vehicle->exhausts.resize(exhaust_count);
	for (int i = 0; i < exhaust_count; i++) {
		vehicle->exhausts[i].transform = ReadTransform();
		vehicle->exhausts[i].strength = ReadFloat();
	}

	int vital_count = ReadInt();
	vehicle->vitals.resize(vital_count);
	for (int i = 0; i < vital_count; i++) {
		vehicle->vitals[i].body_handle = ReadInt();
		vehicle->vitals[i].position = ReadVector();
		vehicle->vitals[i].z_f32 = ReadFloat();
		vehicle->vitals[i].shape_handle = ReadInt();
	}
	vehicle->bounds_dist = ReadFloat();
	vehicle->noroll = ReadBool();
	vehicle->brokenthreshold = ReadFloat();
	vehicle->smokeintensity = ReadFloat();
	return vehicle;
}

Wheel* TDBIN::ReadWheel() {
	Wheel* wheel = new Wheel();
	wheel->vehicle = ReadInt();
	wheel->vehicle_body = ReadInt();
	wheel->body = ReadInt();
	wheel->shape = ReadInt();
	wheel->ground_shape = ReadInt();
	for (int i = 0; i < 3; i++)
		wheel->ground_voxel_pos[i] = ReadInt();
	wheel->z_u8 = ReadByte();
	wheel->transform = ReadTransform();
	wheel->transform2 = ReadTransform();
	wheel->steer = ReadFloat();
	wheel->drive = ReadFloat();
	for (int i = 0; i < 2; i++)
		wheel->travel[i] = ReadFloat();
	wheel->radius = ReadFloat();
	wheel->width = ReadFloat();
	wheel->angular_speed = ReadFloat();
	wheel->z_f32_1 = ReadFloat();
	wheel->z_f32_2 = ReadFloat();
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
	screen->enabled = ReadBool();
	screen->interactive = ReadBool();
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
		trigger->polygon_vertices[i].x = ReadFloat();
		trigger->polygon_vertices[i].y = ReadFloat();
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
	script->variables_count = ReadInt();
	script->variables = ReadLuaTable();

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
		script->transitions[i].time = ReadFloat();
		script->transitions[i].z1_f32 = ReadFloat();
		script->transitions[i].z2_f32 = ReadFloat();
	}
	return script;
}

void* TDBIN::ReadEntityKind(uint8_t type) {
	switch (type) {
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
			printf("[ERROR] Invalid entity type: %d\n", type);
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
	player->bluetide_timer = ReadFloat();
	player->bluetide_power = ReadFloat();
}

void TDBIN::ReadEnvironment() {
	Environment* environment = &scene.environment;
	Skybox* skybox = &environment->skybox;

	skybox->texture = ReadString();
	skybox->tint = ReadColor();
	skybox->brightness = ReadFloat();
	skybox->rot = ReadFloat();
	skybox->sun.tint_brightness = ReadVector();
	skybox->sun.colortint = ReadColor();
	skybox->sun.dir = ReadVector();
	skybox->sun.brightness = ReadFloat();
	skybox->sun.spread = ReadFloat();
	skybox->sun.length = ReadFloat();
	skybox->sun.fogscale = ReadFloat();
	skybox->sun.glare = ReadFloat();
	skybox->auto_sun_dir = ReadBool();
	skybox->constant = ReadColor();
	skybox->ambient = ReadFloat();
	skybox->ambientexponent = ReadFloat();

	for (int i = 0; i < 2; i++)
		environment->exposure[i] = ReadFloat();
	environment->brightness = ReadFloat();

	Fog* fog = &environment->fog;
	fog->color = ReadColor();
	fog->start = ReadFloat();
	fog->distance = ReadFloat();
	fog->amount = ReadFloat();
	fog->exponent = ReadFloat();

	Env::Water* water = &environment->water;
	water->wetness = ReadFloat();
	water->puddleamount = ReadFloat();
	water->puddlesize = ReadFloat();
	water->rain = ReadFloat();

	environment->nightlight = ReadBool();
	environment->ambience.path = ReadString();
	environment->ambience.volume = ReadFloat();
	environment->slippery = ReadFloat();
	environment->fogscale = ReadFloat();

	Snow* snow = &environment->snow;
	snow->dir = ReadVector();
	snow->spread = ReadFloat();
	snow->amount = ReadFloat();
	snow->speed = ReadFloat();
	snow->onground = ReadBool();

	environment->wind = ReadVector();
	environment->waterhurt = ReadFloat();
}

void TDBIN::parse() {
	for (int i = 0; i < 5; i++)
		scene.magic[i] = ReadByte();

	for (int i = 0; i < 3; i++)
		scene.version[i] = ReadByte();

	scene.level_id = ReadString();
	scene.level_path = ReadString();
	scene.layers = ReadString();
	scene.mod = ReadString();
	scene.aaa1 = ReadInt();
	if (scene.aaa1 != 0xAAA1) {
		printf("[ERROR] Invalid Battery Size: %08X\n", scene.aaa1);
		exit(EXIT_FAILURE);
	}

	int entries = ReadInt();
	scene.enabled_mods.resize(entries);
	for (int i = 0; i < entries; i++)
		scene.enabled_mods[i] = ReadRegistry();

	entries = ReadInt();
	scene.spawned_mods.resize(entries);
	for (int i = 0; i < entries; i++)
		scene.spawned_mods[i] = ReadRegistry();

	scene.driven_vehicle = ReadInt();
	scene.shadow_volume = ReadVector();
	scene.spawnpoint = ReadTransform();
	scene.world_body = ReadInt();
	scene.flashlight = ReadInt();
	scene.explosion_lua = ReadInt();
	scene.achievements_lua = ReadInt();

	PostProcessing* postpro = &scene.postpro;
	postpro->brightness = ReadFloat();
	postpro->colorbalance = ReadColor();
	postpro->saturation = ReadFloat();
	postpro->gamma = ReadFloat();
	postpro->bloom = ReadFloat();

	ReadPlayer();
	ReadEnvironment();

	Boundary* boundary = &scene.boundary;
	int vertex_count = ReadInt();
	boundary->vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		boundary->vertices[i].x = ReadFloat();
		boundary->vertices[i].y = ReadFloat();
	}
	boundary->padleft = ReadFloat();
	boundary->padtop = ReadFloat();
	boundary->padright = ReadFloat();
	boundary->padbottom = ReadFloat();
	boundary->maxheight = ReadFloat();

	int fire_count = ReadInt();
	scene.fires.resize(fire_count);
	for (int i = 0; i < fire_count; i++)
		scene.fires[i] = ReadFire();

	int palette_count = ReadInt();
	scene.palettes.resize(palette_count);
	for (int i = 0; i < palette_count; i++)
		scene.palettes[i] = ReadPalette();

	entries = ReadInt();
	scene.registry.resize(entries);
	for (int i = 0; i < entries; i++)
		scene.registry[i] = ReadRegistry();

	int top_entity_count = ReadInt();
	scene.entities.resize(top_entity_count);
	for (int i = 0; i < top_entity_count; i++) {
		scene.entities[i] = ReadEntity();
		scene.entities[i]->parent = NULL;
	}

	entries = ReadInt();
	scene.z_st1.resize(entries);
	for (int i = 0; i < entries; i++) {
		scene.z_st1[i].z_1 = ReadVector();
		scene.z_st1[i].z_2 = ReadVector();
		scene.z_st1[i].z_3 = ReadFloat();
		scene.z_st1[i].z_4 = ReadFloat();
		scene.z_st1[i].z_5 = ReadInt();
		scene.z_st1[i].z_6 = ReadFloat();
	}

	scene.has_snow = ReadBool();

	entries = ReadInt();
	scene.z_st2.resize(entries);
	for (int i = 0; i < entries; i++) {
		scene.z_st2[i].z_1 = ReadString();
		scene.z_st2[i].z_2 = ReadBool();
	}

	if (fgetc(bin_file) != EOF)
		throw runtime_error("File size mismatch.");
	printf("File parsed successfully!\n");
}

void ParseFile(ConverterParams params) {
	progress = 0.1;
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
	progress = 0.25;
	parser.WriteScene();
	parser.WriteSpawnpoint();
	parser.WriteEnvironment();
	parser.WriteBoundary();
	parser.WritePostProcessing();
	progress = 0.5;
	parser.WriteEntities();
	parser.SaveXML();
	progress = 0.75;
	parser.SaveVoxFiles();
	printf("Map successfully converted!\n");
	progress = 1;
}
