#include <math.h>
#include <stdio.h>

#include "parser.h"

void PrintTransform(Transform tr) {
	printf("Pos: %g %g %g\t", tr.pos.x, tr.pos.y, tr.pos.z);
	float bank, heading, attitude;
	QuatToEuler(tr.rot, bank, heading, attitude);
	printf("Rot: %g %g %g\n", bank, heading, attitude);
}

void DEBUG::PrintScene() {
	printf("\n");
	printf("Level name: %s\n", scene.level.c_str());
	printf("\n");

	printf("Scene\n");
	printf("Version: %d.%d.%d\n", scene.version[0], scene.version[1], scene.version[2]);
	printf("ShadowVolume: %g %g %g\n", scene.shadowVolume[0], scene.shadowVolume[1], scene.shadowVolume[2]);
	printf("\n");
}

void DEBUG::PrintSpawnpoint() {
	printf("Spawnpoint\n");
	PrintTransform(scene.spawnpoint);
	printf("\n");
}

void DEBUG::PrintEnvironment() {
	Skybox* skybox = &scene.environment.skybox;
	Fog* fog = &scene.environment.fog;
	EnvWater* water = &scene.environment.water;
	Snow* snow = &scene.environment.snow;

	printf("Environment\n");
	printf("skybox: %s\n", skybox->texture.c_str());
	printf("skyboxtint: %g %g %g %g\n", skybox->tint.r, skybox->tint.g, skybox->tint.b, skybox->tint.a);
	printf("skyboxbrightness: %g\n", skybox->brightness);
	printf("skyboxrot: %g\n", skybox->rot);
	printf("constant: %g %g %g\n", skybox->constant.r, skybox->constant.g, skybox->constant.b);
	printf("ambient: %g\n", skybox->ambient);
	printf("ambientexponent: %g\n", skybox->ambientexponent);
	printf("fogColor: %g %g %g %g\n", fog->color.r, fog->color.g, fog->color.b, fog->color.a);
	printf("fogParams: %g %g %g %g\n", fog->start, fog->start + fog->distance, fog->amount, fog->exponent);
	printf("sunBrightness: %g\n", skybox->sun.brightness);
	printf("sunColorTint: %g %g %g %g\n", skybox->sun.colorTint.r, skybox->sun.colorTint.g, skybox->sun.colorTint.b, skybox->sun.colorTint.a);
	printf("sunDir: %g %g %g\n", skybox->sun.dir[0], skybox->sun.dir[1], skybox->sun.dir[2]);
	printf("sunSpread: %g\n", skybox->sun.spread);
	printf("sunLength: %g\n", skybox->sun.length);
	printf("sunFogScale: %g\n", skybox->sun.fogScale);
	printf("sunGlare: %g\n", skybox->sun.glare);
	printf("exposure: %g %g\n", scene.environment.exposure[0], scene.environment.exposure[1]);
	printf("brightness: %g\n", scene.environment.brightness);
	printf("wetness: %g\n", water->wetness);
	printf("puddleamount: %g\n", water->puddleamount);
	printf("puddlesize: %g\n", 0.01 / water->puddlesize);
	printf("rain: %g\n", water->rain);
	printf("nightlight: %s\n", scene.environment.nightlight ? "true" : "false");
	printf("ambience: %s %g\n", scene.environment.ambience.path.c_str(), scene.environment.ambience.volume);
	printf("fogscale: %g\n", scene.environment.fogscale);
	printf("slippery: %g\n", scene.environment.slippery);
	printf("waterhurt: %g\n", scene.environment.waterhurt);
	printf("snowdir: %g %g %g %g\n", snow->dir[0], snow->dir[1], snow->dir[2], snow->spread);
	printf("snowamount: %g\n", snow->amount);
	printf("snowonground: %s\n", snow->onground ? "true" : "false");
	printf("wind: %g %g %g\n", scene.environment.wind[0], scene.environment.wind[1], scene.environment.wind[2]);
	printf("\n");
}

void DEBUG::PrintBoundary() {
	printf("Boundary\n");
	int vertex_count = scene.boundary.vertices.getSize();
	for (int i = 0; i < vertex_count; i++)
		printf("Vertex: %g %g\n", scene.boundary.vertices[i].pos[0], scene.boundary.vertices[i].pos[1]);
	printf("Padleft: %g\n", -scene.boundary.padleft);
	printf("Padright: %g\n", scene.boundary.padright);
	printf("Padtop: %g\n", -scene.boundary.padtop);
	printf("Padbottom: %g\n", scene.boundary.padbottom);
	printf("\n");
}

void DEBUG::PrintPostProcessing() {
	printf("PostProcessing\n");
	printf("Saturation: %g\n", scene.postpro.saturation);
	printf("Color Balance: %g %g %g %g\n", scene.postpro.colorbalance.r, scene.postpro.colorbalance.g, scene.postpro.colorbalance.b, scene.postpro.colorbalance.a);
	printf("Brightness: %g\n", scene.postpro.brightness);
	printf("Gamma: %g\n", scene.postpro.gamma);
	printf("Bloom: %g\n", scene.postpro.bloom);
	printf("\n");
}

void DEBUG::PrintEntities() {
	for (unsigned int i = 0; i < scene.entities.getSize(); i++)
		PrintEntity(scene.entities[i]);
}

void DEBUG::PrintEntity(Entity* entity) {
	if (entity->kind_byte != KindVehicle) { // TODO: Remove!!!
		for (unsigned int i = 0; i < entity->children.getSize(); i++)
			PrintEntity(entity->children[i]);
		return;
	}

	printf("Entity %s %d\n", EntityKindName[entity->kind_byte], entity->handle);
	if (entity->tags.getSize() > 0) {
		printf("Tags: ");
		for (unsigned int i = 0; i < entity->tags.getSize(); i++) {
			printf("%s", entity->tags[i].name.c_str());
			if (entity->tags[i].value.length() > 0)
				printf("=%s", entity->tags[i].value.c_str());
			printf(" ");
		}
		printf("\n");
	}

	switch (entity->kind_byte) {
		case KindBody: {
			Body* body = (Body*)entity->kind;
			PrintTransform(body->transform);
			printf("Dynamic: %s\n", body->dynamic ? "true" : "false");
		}
			break;
		case KindShape: {
			Shape* shape = (Shape*)entity->kind;
			PrintTransform(shape->transform);
			printf("Texture: %d %g\n", shape->texture_tile, shape->texture_weight);
			printf("Blendtexture: %d %g\n", shape->blendtexture_tile, shape->blendtexture_weight);
			printf("Density: %g\n", shape->density);
			printf("Strength: %g\n", shape->strength);
			//printf("Collide: %s\n", shape->collide ? "true" : "false"); // TODO: Shape collide
			printf("Prop: false\n");
			//printf("File: %s\n", shape->file);
			//printf("Object: %s\n", shape->object);
			printf("Scale: %g\n", 10.0 * shape->scale);
		}
			break;
		case KindLight: {
			Light* light = (Light*)entity->kind;
			PrintTransform(light->transform);
			if (light->type == Sphere)
				printf("Type: Sphere\n");
			else if (light->type == Capsule)
				printf("Type: Capsule\n");
			else if (light->type == Cone)
				printf("Type: Cone\n");
			else if (light->type == Area)
				printf("Type: Area\n");
			printf("Color: %g %g %g %g\n", pow(light->color.r, 0.454545), pow(light->color.g, 0.454545), pow(light->color.b, 0.454545), light->color.a);
			printf("Scale: %g\n", light->scale);
			if (light->type == Cone) {
				printf("Angle: %g\n", 2.0 * deg(acos(light->angle)));
				printf("Penumbra: %g\n", 2.0 * deg(acos(light->angle) - acos(light->penumbra)));
			}
			if (light->type == Area)
				printf("Size: %g %g\n", 2.0 * light->area_size[0], 2.0 * light->area_size[1]);
			else if (light->type == Capsule)
				printf("Size: %g %g\n", 2.0 * light->capsule_size, light->size);
			else
				printf("Size: %g\n", light->size);
			printf("Reach: %g\n", light->reach);
			printf("Unshadowed: %g\n", light->unshadowed);
			printf("Fogscale: %g\n", light->fogscale);
			printf("Fogiter: %g\n", light->fogiter);
			printf("Sound: %s %g\n", light->sound.path.c_str(), light->sound.volume);
			printf("Glare: %g\n", light->glare);
		}
			break;
		case KindLocation: {
			Location* location = (Location*)entity->kind;
			PrintTransform(location->transform);
		}
			break;
		case KindWater: {
			Water* water = (Water*)entity->kind;
			PrintTransform(water->transform);
			printf("Depth: %g\n", water->depth);
			printf("Wave: %g\n", water->wave);
			printf("Ripple: %g\n", water->ripple);
			printf("Motion: %g\n", water->motion);
			printf("Foam: %g\n", water->foam);
			printf("Color: %g %g %g %g\n", water->color.r, water->color.g, water->color.b, water->color.a);

			int vertex_count = water->water_vertices.getSize();
			for (int i = 0; i < vertex_count; i++)
				printf("Vertex: %g %g\n", water->water_vertices[i].pos[0], water->water_vertices[i].pos[1]);
		}
			break;
		case KindJoint: {
			Joint* joint = (Joint*)entity->kind;
			if (joint->type == _Rope) {
				printf("Rope\n");
				printf("Size: %g\n", joint->size);
				printf("Color: %g %g %g %g\n", joint->rope.color.r, joint->rope.color.g, joint->rope.color.b, joint->rope.color.a);
				// printf("Slack: %g\n", joint->rope.slack); // TODO: Joint Rope slack
				printf("Strength: %g\n", joint->rope.strength);
				printf("Maxstretch: %g\n", joint->rope.maxstretch);

				int knot_count = joint->rope.knots.getSize();
				for (int i = 0; i < knot_count; i++) {
					printf("Location From: %g %g %g\n", joint->rope.knots[i].from[0], joint->rope.knots[i].from[2], joint->rope.knots[i].from[2]);
					printf("Location To: %g %g %g\n", joint->rope.knots[i].to[0], joint->rope.knots[i].to[2], joint->rope.knots[i].to[2]);
				}
			} else {
				if (joint->type == Ball)
					printf("Type: Ball\n");
				else if (joint->type == Hinge)
					printf("Type: Hinge\n");
				else if (joint->type == Prismatic)
					printf("Type: Prismatic\n");

				printf("Size: %g\n", joint->size);
				if (joint->type != Prismatic) {
					printf("Rotstrength: %g\n", joint->rotstrength);
					printf("Rotspring: %g\n", joint->rotspring);
				}
				printf("Collide: %s\n", joint->collide ? "true" : "false");
				if (joint->type == Hinge)
					printf("Limits: %g %g\n", deg(joint->limits[0]), deg(joint->limits[1]));
				else if (joint->type == Prismatic)
					printf("Limits: %g %g\n", joint->limits[0], joint->limits[1]);

				printf("Sound: %s\n", joint->sound ? "true" : "false");
				if (joint->type == Prismatic)
					printf("Autodisable: %s\n", joint->autodisable ? "true" : "false");
			}
		}
			break;
		case KindVehicle: {
			Vehicle* vehicle = (Vehicle*)entity->kind;
			PrintTransform(vehicle->transform);
			//printf("Driven: %s\n", vehicle->driven ? "true" : "false"); // TODO: Vehicle driven

			printf("Sound: %s %g\n", vehicle->properties.sound.name.c_str(), vehicle->properties.sound.pitch);
			printf("Spring: %g\n", vehicle->properties.spring);
			printf("Damping: %g\n", vehicle->properties.damping);
			printf("Topspeed: %g\n", 3.6 * vehicle->properties.topspeed);
			printf("Acceleration: %g\n", vehicle->properties.acceleration);
			printf("Strength: %g\n", vehicle->properties.strength);
			printf("Antispin: %g\n", vehicle->properties.antispin);
			printf("Antiroll: %g\n", vehicle->properties.antiroll);
			printf("Difflock: %g\n", vehicle->difflock);
			printf("Steerassist: %g\n", vehicle->properties.steerassist);
			printf("Friction: %g\n", vehicle->properties.friction);

			int exhausts_count = vehicle->exhausts.getSize();
			for (int i = 0; i < exhausts_count; i++) {
				printf("Exhaust: ");
				PrintTransform(vehicle->exhausts[i].transform);
			}
			printf("Exit: %g %g %g\n", vehicle->exit[0], vehicle->exit[1], vehicle->exit[2]);
			printf("Camera: %g %g %g\n", vehicle->camera[0], vehicle->camera[1], vehicle->camera[2]);
			printf("Player: %g %g %g\n", vehicle->player[0], vehicle->player[1], vehicle->player[2]);
			printf("Propeller: %g %g %g\n", vehicle->propeller[0], vehicle->propeller[1], vehicle->propeller[2]);
			int vital_count = vehicle->vitals.getSize();
			for (int i = 0; i < vital_count; i++)
				printf("Vital: %g %g %g\n", vehicle->vitals[i].pos[0], vehicle->vitals[i].pos[1], vehicle->vitals[i].pos[2]);
		}
			break;
		case KindWheel: {
			Wheel* wheel = (Wheel*)entity->kind;
			PrintTransform(wheel->transform);
			printf("Drive: %g\n", wheel->drive);
			printf("Steer: %g\n", wheel->steer);
			printf("Travel: %g %g\n", wheel->travel[0], wheel->travel[1]);
		}
			break;
		case KindScreen: {
			Screen* screen = (Screen*)entity->kind;
			PrintTransform(screen->transform);
			printf("Size: %g %g\n", screen->size[0], screen->size[1]);
			printf("Bulge: %g\n", screen->bulge);
			printf("Resolution: %d %d\n", screen->resolution[0], screen->resolution[1]);
			printf("Script: %s\n", screen->script.c_str());
			printf("Enabled: %s\n", screen->enabled ? "true" : "false");
			printf("Interactive: %s\n", screen->interactive ? "true" : "false");
			printf("Emissive: %g\n", screen->emissive);
			printf("FXRaster: %g\n", screen->fxraster);
			printf("FXCA: %g\n", screen->fxca);
			printf("FXNoice: %g\n", screen->fxnoise);
			//printf("FXGlitch: %g\n", screen->fxglitch); // TODO: Screen fxglitch
		}
			break;
		case KindTrigger: {
			Trigger* trigger = (Trigger*)entity->kind;
			Transform trigger_tr = trigger->transform; // Copy the transform to avoid overwrite
			if (trigger->type == TrBox)
				trigger_tr.pos.x -= trigger->box_size[1];
			PrintTransform(trigger_tr);

			if (trigger->type == TrSphere) {
				printf("Type: Sphere\n");
				printf("Size: %g\n", trigger->sphere_size);
			} else if (trigger->type == TrBox) {
				printf("Type: Box\n");
				printf("Size: %g %g %g\n", 2.0 * trigger->box_size[0], 2.0 * trigger->box_size[1], 2.0 * trigger->box_size[2]);
			} else if (trigger->type == TrPolygon) {
				printf("Type: Polygon\n");
				printf("Size: %g\n", trigger->polygon_size);
				int vertex_count = trigger->polygon_vertices.getSize();
				for (int i = 0; i < vertex_count; i++)
					printf("Vertex: %g %g\n", trigger->polygon_vertices[i].pos[0], trigger->polygon_vertices[i].pos[1]);
			}

			printf("Sound: %s %g\n", trigger->sound.path.c_str(), trigger->sound.volume);
			printf("Sound Ramp: %g\n", trigger->sound.soundramp);
		}
			break;
		case KindScript: {
			Script* script = (Script*)entity->kind;
			printf("File: %s\n", script->file.c_str());
			for (unsigned int i = 0; i < script->params.getSize(); i++)
				printf("param%d: %s=%s\n", i, script->params[i].key.c_str(), script->params[i].value.c_str());
		}
			break;
	}
	if (entity->desc.length() > 0)
		printf("Desc: %s\n", entity->desc.c_str());
	printf("\n");

	for (unsigned int i = 0; i < entity->children.getSize(); i++)
		PrintEntity(entity->children[i]);
}
