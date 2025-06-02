#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <filesystem>

#include "parser.h"
#include "write_scene.h"
#include "zlib_utils.h"

using namespace std::filesystem;

TDBIN::TDBIN() { }

void TDBIN::InitScene(const char* input) {
	int len = strlen(input) + 3; // for "td\0" in ".tdbin"
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
	InitReader(filename);
	printf("Parsing file...\n");
	delete[] filename;
}

TDBIN::~TDBIN() {
	for (unsigned int i = 0; i < scene.entities.getSize(); i++)
		delete scene.entities[i];
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
		p.materials[i].type = ReadByte();
		p.materials[i].rgba = ReadColor();
		p.materials[i].reflectivity = ReadFloat();
		p.materials[i].shinyness = ReadFloat();
		p.materials[i].metalness = ReadFloat();
		p.materials[i].emissive = ReadFloat();
		p.materials[i].is_tint = ReadBool();
	}
	p.has_transparent = ReadBool();
	ReadBuffer(p.black_tint, 4 * 256);
	ReadBuffer(p.yellow_tint, 4 * 256);
	ReadBuffer(p.rgba_tint, 4 * 256);
	return p;
}

Rope* TDBIN::ReadRope() {
	Rope* rope = new Rope();
	rope->color = ReadColor();
	rope->zero = ReadFloat();
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

VehicleProperties TDBIN::ReadVehicleProperties() {
	VehicleProperties properties;
	properties.topspeed = ReadFloat();
	properties.top_speed_clamp = ReadFloat();
	properties.spring = ReadFloat();
	properties.damping = ReadFloat();
	properties.acceleration = ReadFloat();
	properties.strength = ReadFloat();
	properties.friction = ReadFloat();
	properties.max_steer_angle = ReadFloat();
	properties.handbrake = ReadBool();
	properties.antispin = ReadFloat();
	properties.steerassist = ReadFloat();
	properties.assist_multiplier = ReadFloat();
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
	voxels.palette = ReadInt();
	voxels.scale = ReadFloat();

	for (int i = 0; i < 8; i++)
		voxels.light_mask[i] = ReadByte();
	voxels.is_disconnected = ReadByte();
	return voxels;
}

LuaValue TDBIN::ReadLuaValue(LuaType key_type) {
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
		case Table:
			value.Table = ReadLuaTable();
			break;
		case Reference:
			value.Reference = ReadInt();
			break;
		case NIL:
			break;
		default:
			break;
	}
	return value;
}

LuaTable* TDBIN::ReadLuaTable() {
	LuaTable* table = new LuaTable();
	do {
		LuaTableEntry* table_entry = new LuaTableEntry();
		table_entry->key_type = (LuaType)ReadInt();
		if (table_entry->key_type == NIL) {
			table->push_back(table_entry);
			break;
		}
		table_entry->key = ReadLuaValue(table_entry->key_type);
		table_entry->value_type = (LuaType)ReadInt();
		table_entry->value = ReadLuaValue(table_entry->value_type);
		table->push_back(table_entry);
	} while (true);
	return table;
}

Entity* TDBIN::ReadEntity() {
	Entity* entity = new Entity();
	entity->type = ReadByte();

	entity->handle = ReadInt();
	entity_mapping[entity->handle] = entity;
	//printf("Reading %s with handle %d\n", EntityName[entity->type], entity->handle);

	uint8_t tag_count = ReadByte();
	entity->tags.resize(tag_count);
	for (uint8_t i = 0; i < tag_count; i++)
		entity->tags[i] = ReadTag();

	entity->desc = ReadString();
	if (entity->type != Entity::Light && entity->type != Entity::Joint) // Ah, yes... consistency
		entity->flags = ReadWord();
	entity->self = ReadEntityType(entity->type);

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

	shape->origin = ReadByte();
	if (tdbin_version >= VERSION_1_6_0)
		shape->animator = ReadInt();
	return shape;
}

Light* TDBIN::ReadLight() {
	Light* light = new Light();
	light->enabled = ReadBool();
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
	light->flickering = ReadFloat();
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
	water->vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++)
		water->vertices[i] = ReadVertex();
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
		joint->axes[i] = ReadVector();
	joint->connected = ReadBool();
	joint->collide = ReadBool();
	joint->rotstrength = ReadFloat();
	joint->rotspring = ReadFloat();
	joint->hinge_rot = ReadQuat();
	for (int i = 0; i < 2; i++)
		joint->limits[i] = ReadFloat();
	joint->max_velocity = ReadFloat();
	joint->strength = ReadFloat();
	joint->size = ReadFloat();
	joint->sound = ReadBool();
	joint->autodisable = ReadBool();
	joint->connection_strength = ReadFloat();
	joint->disconnect_dist = ReadFloat();
	if (joint->type == Joint::_Rope)
		joint->rope = ReadRope();
	else
		joint->rope = NULL;
	return joint;
}

Vehicle* TDBIN::ReadVehicle() {
	Vehicle* vehicle = new Vehicle();
	vehicle->body = ReadInt();
	vehicle->transform = ReadTransform();
	vehicle->transform2 = ReadTransform();

	int wheel_count = ReadInt();
	vehicle->wheels.resize(wheel_count);
	for (int i = 0; i < wheel_count; i++)
		vehicle->wheels[i] = ReadInt();

	vehicle->properties = ReadVehicleProperties();

	vehicle->camera = ReadVector();
	vehicle->player = ReadVector();
	vehicle->exit = ReadVector();
	vehicle->propeller = ReadVector();
	vehicle->difflock = ReadFloat();
	vehicle->health = ReadFloat();
	vehicle->main_voxel_count = ReadInt();
	vehicle->braking = ReadBool();
	vehicle->passive_brake = ReadFloat();

	int ref_count = ReadInt();
	vehicle->bodies.resize(ref_count);
	for (int i = 0; i < ref_count; i++)
		vehicle->bodies[i] = ReadInt();

	int exhaust_count = ReadInt();
	vehicle->exhausts.resize(exhaust_count);
	for (int i = 0; i < exhaust_count; i++) {
		vehicle->exhausts[i].transform = ReadTransform();
		vehicle->exhausts[i].strength = ReadFloat();
	}

	int vital_count = ReadInt();
	vehicle->vitals.resize(vital_count);
	for (int i = 0; i < vital_count; i++) {
		vehicle->vitals[i].body = ReadInt();
		vehicle->vitals[i].position = ReadVector();
		vehicle->vitals[i].radius = ReadFloat();
		vehicle->vitals[i].nearby_voxels = ReadInt();
	}

	if (tdbin_version >= VERSION_1_6_0) {
		int anim_count = ReadInt();
		vehicle->locations.resize(anim_count);
		for (int i = 0; i < anim_count; i++) {
			vehicle->locations[i].name = ReadString();
			vehicle->locations[i].transform = ReadTransform();
			vehicle->locations[i].handle = ReadInt();
		}
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
	wheel->on_ground = ReadByte();
	wheel->transform = ReadTransform();
	wheel->transform2 = ReadTransform();
	wheel->steer = ReadFloat();
	wheel->drive = ReadFloat();
	for (int i = 0; i < 2; i++)
		wheel->travel[i] = ReadFloat();
	wheel->radius = ReadFloat();
	wheel->width = ReadFloat();
	wheel->angular_speed = ReadFloat();
	wheel->stance = ReadFloat();
	wheel->vertical_offset = ReadFloat();
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
	for (int i = 0; i < vertex_count; i++)
		trigger->polygon_vertices[i] = ReadVertex();
	trigger->sound.path = ReadString();
	trigger->sound.ramp = ReadFloat();
	trigger->sound.type = ReadByte();
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

	script->tick_time = ReadFloat();
	script->update_time = ReadFloat();
	script->variables_count = ReadInt();
	script->variables = ReadLuaTable();

	int entities = ReadInt();
	script->entities.resize(entities);
	for (int i = 0; i < entities; i++)
		script->entities[i] = ReadInt();

	int sound_count = ReadInt();
	script->sounds.resize(sound_count);
	for (int i = 0; i < sound_count; i++) {
		script->sounds[i].type = ReadInt();
		script->sounds[i].name = ReadString();
	}

	int transition_count = ReadInt();
	script->transitions.resize(transition_count);
	for (int i = 0; i < transition_count; i++) {
		script->transitions[i].variable = ReadString();
		script->transitions[i].transition = ReadByte();
		script->transitions[i].target_time = ReadFloat();
		script->transitions[i].current_time = ReadFloat();
		script->transitions[i].current_value = ReadFloat();
		script->transitions[i].target_value = ReadFloat();
	}
	return script;
}

Animator* TDBIN::ReadAnimator() {
	int entries = 0;
	uint8_t* buffer = NULL;
	Animator* animator = new Animator();
	animator->transform = ReadTransform();
	animator->path = ReadString();

	ReadBool();

	entries = ReadInt();
	for (int i = 0; i < entries; i++) {
		ReadInt();
		ReadInt();
		ReadInt();
		ReadString();
	}

	entries = ReadInt();
	for (int i = 0; i < entries; i++) {
		ReadTransform();
		ReadVertex();
		ReadVertex();
		ReadFloat();
		ReadFloat();
		ReadByte();
		ReadByte();
		ReadInt();
		ReadInt();
		ReadInt();
		ReadInt();
		ReadQuat();
		ReadVector();
		ReadVector();
		ReadVector();
		ReadVector();
		ReadInt();
		ReadInt();
	}

	entries = ReadInt();
	for (int i = 0; i < entries; i++) {
		ReadInt();
		ReadTransform();
		ReadFloat();
		ReadFloat();
		ReadInt();
		ReadInt();
		ReadByte();
		ReadByte();
		ReadByte();
		ReadByte();
		ReadBool();
		ReadVoxels();
	}

	entries = ReadInt();
	for (int i = 0; i < entries; i++) {
		ReadString();
		uint8_t st_buffer[56];
		ReadBuffer(st_buffer, 56);
	}

	entries = ReadInt();
	for (int i = 0; i < entries; i++) {
		ReadString();
		uint8_t st_buffer[128];
		ReadBuffer(st_buffer, 128);
	}

	ReadInt();

	entries = ReadInt();
	for (int i = 0; i < entries; i++) {
		ReadString();
		uint8_t st_buffer[72];
		ReadBuffer(st_buffer, 72);
	}

	ReadInt();

	entries = ReadInt();
	buffer = new uint8_t[8 * entries];
	ReadBuffer(buffer, 8 * entries);
	delete[] buffer;

	entries = ReadInt();
	for (int i = 0; i < entries; i++)
		ReadString();

	entries = ReadInt();
	buffer = new uint8_t[28 * entries];
	ReadBuffer(buffer, 28 * entries);
	delete[] buffer;

	entries = ReadInt();
	buffer = new uint8_t[28 * entries];
	ReadBuffer(buffer, 28 * entries);
	delete[] buffer;

	entries = ReadInt();
	buffer = new uint8_t[4 * entries];
	ReadBuffer(buffer, 4 * entries);
	delete[] buffer;

	entries = ReadInt();
	for (int i = 0; i < entries; i++) {
		ReadString();
		ReadTransform();
	}
	return animator;
}

void* TDBIN::ReadEntityType(uint8_t type) {
	switch (type) {
		case Entity::Body:
			return ReadBody();
		case Entity::Shape:
			return ReadShape();
		case Entity::Light:
			return ReadLight();
		case Entity::Location:
			return ReadLocation();
		case Entity::Water:
			return ReadWater();
		case Entity::Joint:
			return ReadJoint();
		case Entity::Vehicle:
			return ReadVehicle();
		case Entity::Wheel:
			return ReadWheel();
		case Entity::Screen:
			return ReadScreen();
		case Entity::Trigger:
			return ReadTrigger();
		case Entity::Script:
			return ReadScript();
		case Entity::Animator:
			return ReadAnimator();
		default:
			printf("[ERROR] Invalid entity type: %d\n", (uint8_t)type);
			exit(EXIT_FAILURE);
			return NULL;
	}
}

void TDBIN::ReadPostProcessing() {
	PostProcessing* postpro = &scene.postpro;
	postpro->brightness = ReadFloat();
	postpro->colorbalance = ReadColor();
	postpro->saturation = ReadFloat();
	postpro->gamma = ReadFloat();
	postpro->bloom = ReadFloat();
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
	if (tdbin_version >= VERSION_1_6_0)
		player->animator = ReadFloat();
}

void TDBIN::ReadEnvironment() {
	Environment* environment = &scene.environment;
	Skybox* skybox = &environment->skybox;
	Sun* sun = &skybox->sun;

	skybox->texture = ReadString();
	skybox->tint = ReadColor();
	skybox->brightness = ReadFloat();
	skybox->rot = ReadFloat();
	sun->tint_brightness = ReadVector();
	sun->colortint = ReadColor();
	sun->dir = ReadVector();
	sun->brightness = ReadFloat();
	sun->spread = ReadFloat();
	sun->length = ReadFloat();
	sun->fogscale = ReadFloat();
	sun->glare = ReadFloat();
	sun->auto_dir = ReadBool();
	skybox->constant = ReadColor();
	skybox->ambient = ReadFloat();
	skybox->ambientexponent = ReadFloat();

	for (int i = 0; i < 2; i++)
		environment->exposure[i] = ReadFloat();
	environment->brightness = ReadFloat();

	Fog* fog = &environment->fog;
	if (tdbin_version >= VERSION_1_6_0)
		fog->type = ReadByte();
	else
		fog->type = 0;
	fog->color = ReadColor();
	fog->start = ReadFloat();
	fog->distance = ReadFloat();
	fog->amount = ReadFloat();
	fog->exponent = ReadFloat();
	if (tdbin_version >= VERSION_1_6_0)
		fog->height_offset = ReadFloat();
	else
		fog->height_offset = 0.0f;

	Environment::Water* water = &environment->water;
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

	if (tdbin_version >= VERSION_1_6_3)
		environment->lensdirt = ReadString();
	else
		environment->lensdirt = "";
}

void TDBIN::parse() {
	for (int i = 0; i < 5; i++)
		scene.magic[i] = ReadByte();
	for (int i = 0; i < 3; i++)
		scene.version[i] = ReadByte();
	tdbin_version = scene.version[0] * 100 + scene.version[1] * 10 + scene.version[2];

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
	if (tdbin_version >= VERSION_1_6_0)
		scene.characters_lua = ReadInt();

	ReadPostProcessing();
	ReadPlayer();
	ReadEnvironment();

	Boundary* boundary = &scene.boundary;
	int vertex_count = ReadInt();
	boundary->vertices.resize(vertex_count);
	for (int i = 0; i < vertex_count; i++)
		boundary->vertices[i] = ReadVertex();
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
	scene.projectiles.resize(entries);
	for (int i = 0; i < entries; i++) {
		scene.projectiles[i].origin = ReadVector();
		scene.projectiles[i].direction = ReadVector();
		scene.projectiles[i].dist = ReadFloat();
		scene.projectiles[i].max_dist = ReadFloat();
		scene.projectiles[i].type = ReadInt();
		scene.projectiles[i].strength = ReadFloat();
	}

	scene.has_snow = ReadBool();

	entries = ReadInt();
	scene.assets.resize(entries);
	for (int i = 0; i < entries; i++) {
		scene.assets[i].folder = ReadString();
		scene.assets[i].do_override = ReadBool();
	}
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
	printf("Generating XML file...\n");
	parser.WriteScene();
	parser.WriteSpawnpoint();
	parser.WriteEnvironment();
	parser.WriteBoundary();
	parser.WritePostProcessing();
	progress = 0.5;
	parser.WriteEntities();
	printf("Saving XML file...\n");
	parser.SaveXML();
	progress = 0.75;
	printf("Saving VOX files...\n");
	parser.SaveVoxFiles();
	printf("Map successfully converted!\n");
	progress = 1;
}
