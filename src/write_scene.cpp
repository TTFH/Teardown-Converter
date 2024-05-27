#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "scene.h"
#include "entity.h"
#include "math_utils.h"
#include "vox_writer.h"
#include "xml_writer.h"
#include "write_scene.h"
#include "../lib/tinyxml2.h"

WriteXML::WriteXML(ConverterParams params) : params(params) {
	init(params.bin_path.c_str());
}

WriteXML::~WriteXML() {
	for (map<uint32_t, MV_FILE*>::iterator it = vox_files.begin(); it != vox_files.end(); it++)
		delete it->second;
	vox_files.clear();
}

void WriteXML::WriteScene() {
	string version = to_string(scene.version[0]) + "." + to_string(scene.version[1]) + "." + to_string(scene.version[2]);
	xml.AddStrAttribute(xml.GetScene(), "version", version);
	xml.AddVectorAttribute(xml.GetScene(), "shadowVolume", scene.shadow_volume, "100 25 100");
}

void WriteXML::WriteTransform(XMLElement* element, Transform tr) {
	xml.AddVectorAttribute(element, "pos", tr.pos, "0 0 0");
	float rotation[3];
	QuatToEuler(tr.rot, rotation[0], rotation[1], rotation[2]);
	xml.AddFloatNAttribute(element, "rot", rotation, 3, "0 0 0");
}

void WriteXML::WriteVertices(XMLElement* parent, const Vec<Vertex> &vertices) {
	for (unsigned int i = 0; i < vertices.getSize(); i++) {
		XMLElement* vertex = xml.CreateElement("vertex");
		xml.AddElement(parent, vertex);
		xml.AddFloat2Attribute(vertex, "pos", vertices[i].x, vertices[i].y);
	}
}

void WriteXML::WriteSpawnpoint() {
	XMLElement* spawnpoint = xml.CreateElement("spawnpoint");
	xml.AddElement(xml.GetScene(), spawnpoint);
	scene.spawnpoint.rot = scene.spawnpoint.rot * QuatEuler(0, 180, 0);
	WriteTransform(spawnpoint, scene.spawnpoint);
}

void WriteXML::WriteEnvironment() {
	Skybox* skybox = &scene.environment.skybox;
	Fog* fog = &scene.environment.fog;
	Env::Water* water = &scene.environment.water;
	Snow* snow = &scene.environment.snow;

	XMLElement* environment = xml.CreateElement("environment");
	xml.AddElement(xml.GetScene(), environment);

	string skybox_texture = skybox->texture;
	string prefix = "data/env/";
	if (skybox_texture.find(prefix) == 0)
		skybox_texture = skybox_texture.substr(prefix.size());

	if (!params.dlc_id.empty()) {
		prefix = "dlcs/" + params.dlc_id + "/env/";
		if (skybox_texture.find(prefix) == 0)
			skybox_texture = skybox_texture.substr(prefix.size());
	}

	xml.AddStrAttribute(environment, "skybox", skybox_texture, "cloudy.dds");
	xml.AddColorAttribute(environment, "skyboxtint", skybox->tint, "1 1 1");
	xml.AddFloatAttribute(environment, "skyboxbrightness", skybox->brightness, "1");
	xml.AddFloatAttribute(environment, "skyboxrot", deg(skybox->rot), "0");
	xml.AddColorAttribute(environment, "constant", skybox->constant, "0.003 0.003 0.003");
	xml.AddFloatAttribute(environment, "ambient", skybox->ambient, "1");
	xml.AddFloatAttribute(environment, "ambientexponent", skybox->ambientexponent, "1.3");
	xml.AddColorAttribute(environment, "fogColor", fog->color, "1 1 1");
	xml.AddFloat4Attribute(environment, "fogParams", fog->start, fog->start + fog->distance, fog->amount, fog->exponent, "40 100 0.9 4");
	xml.AddFloatAttribute(environment, "sunBrightness", skybox->sun.brightness, "0");
	xml.AddColorAttribute(environment, "sunColorTint", skybox->sun.colortint, "1 1 1");
	if (skybox->auto_sun_dir)
		xml.AddStrAttribute(environment, "sunDir", "auto");
	else
		xml.AddVectorAttribute(environment, "sunDir", skybox->sun.dir);
	xml.AddFloatAttribute(environment, "sunSpread", skybox->sun.spread, "0");
	xml.AddFloatAttribute(environment, "sunLength", skybox->sun.length, "32");
	xml.AddFloatAttribute(environment, "sunFogScale", skybox->sun.fogscale, "1");
	xml.AddFloatAttribute(environment, "sunGlare", skybox->sun.glare, "1");
	xml.AddFloatNAttribute(environment, "exposure", scene.environment.exposure, 2, "0 10");
	xml.AddFloatAttribute(environment, "brightness", scene.environment.brightness, "1");
	xml.AddFloatAttribute(environment, "wetness", water->wetness, "0");
	xml.AddFloatAttribute(environment, "puddleamount", water->puddleamount, "0");
	xml.AddFloatAttribute(environment, "puddlesize", 0.01 / water->puddlesize, "0.5");
	xml.AddFloatAttribute(environment, "rain", water->rain, "0");
	xml.AddBoolAttribute(environment, "nightlight", scene.environment.nightlight, true);
	xml.AddStrFloatAttribute(environment, "ambience", scene.environment.ambience.path, scene.environment.ambience.volume, "outdoor/field.ogg");
	xml.AddFloatAttribute(environment, "fogscale", scene.environment.fogscale, "1");
	xml.AddFloatAttribute(environment, "slippery", scene.environment.slippery, "0");
	xml.AddFloatAttribute(environment, "waterhurt", scene.environment.waterhurt, "0");
	xml.AddFloat4Attribute(environment, "snowdir", snow->dir.x, snow->dir.y, snow->dir.z, snow->spread, "0 -1 0 0.2");
	xml.AddFloat2Attribute(environment, "snowamount", snow->amount, snow->speed, "0 0");
	xml.AddBoolAttribute(environment, "snowonground", snow->onground && !params.remove_snow, false);
	xml.AddVectorAttribute(environment, "wind", scene.environment.wind, "0 0 0");

	params.remove_snow = params.remove_snow && snow->onground; // Only remove snow if there is snow to remove
}

void WriteXML::WriteBoundary() {
	int vertex_count = scene.boundary.vertices.getSize();
	if (vertex_count == 0)
		return;

	XMLElement* boundary = xml.CreateElement("boundary");
	xml.AddElement(xml.GetScene(), boundary);

	xml.AddFloatAttribute(boundary, "padleft", -scene.boundary.padleft, "5");
	xml.AddFloatAttribute(boundary, "padright", scene.boundary.padright, "5");
	xml.AddFloatAttribute(boundary, "padtop", -scene.boundary.padtop, "5");
	xml.AddFloatAttribute(boundary, "padbottom", scene.boundary.padbottom, "5");
	xml.AddFloatAttribute(boundary, "maxheight", scene.boundary.maxheight, "0");

	WriteVertices(boundary, scene.boundary.vertices);
}

void WriteXML::WritePostProcessing() {
	XMLElement* postprocessing = xml.CreateElement("postprocessing");
	xml.AddElement(xml.GetScene(), postprocessing);

	xml.AddFloatAttribute(postprocessing, "saturation", scene.postpro.saturation, "1");
	xml.AddColorAttribute(postprocessing, "colorbalance", scene.postpro.colorbalance, "1 1 1");
	xml.AddFloatAttribute(postprocessing, "brightness", scene.postpro.brightness, "1");
	xml.AddFloatAttribute(postprocessing, "gamma", scene.postpro.gamma, "1");
	xml.AddFloatAttribute(postprocessing, "bloom", scene.postpro.bloom, "1");
}

void WriteXML::SaveXML() {
	xml.ClearEmptyGroups();
	string main_xml_path = params.map_folder + (params.legacy_format ? "custom.xml" : "main.xml");
	xml.SaveFile(main_xml_path.c_str());
}

void WriteXML::SaveVoxFiles() {
	for (map<uint32_t, MV_FILE*>::iterator it = vox_files.begin(); it != vox_files.end(); it++)
		it->second->SaveModel(params.compress_vox);
	for (vector<MV_FILE*>::iterator it = compound_files.begin(); it != compound_files.end(); it++)
		(*it)->SaveModel(params.compress_vox);
}

void WriteXML::WriteEntities() {
	xml.PostInit();

	for (unsigned int i = 0; i < scene.entities.getSize(); i++)
		WriteEntity(xml.GetScene(), scene.entities[i]);

	if (scene.driven_vehicle != 0) {
		XMLElement* xml_vehicle = xml.GetNode(scene.driven_vehicle);
		if (xml_vehicle != NULL)
			xml.AddBoolAttribute(xml_vehicle, "driven", true, false);
	}

	// Iterate in reverse order, so parent scripts comes after their childrens
	for (int i = scene.entities.getSize() - 1; i >= 0; i--)
		WriteEntity2ndPass(scene.entities[i]);
}

// TODO: refactor
void WriteXML::WriteShape(XMLElement* &parent_element, XMLElement* &entity_element, Entity* entity) {
	assert(entity->type == KindShape);
	Shape* shape = static_cast<Shape*>(entity->self);
	uint32_t handle = entity->handle;

	int sizex = shape->voxels.size[0];
	int sizey = shape->voxels.size[1];
	int sizez = shape->voxels.size[2];
	unsigned int volume = sizex * sizey * sizez;
	if (volume == 0) {
		entity_element = NULL; // Ignore empty shapes
		return;
	}

	shape->old_transform = shape->transform;
	Palette palette = scene.palettes[shape->palette];
	bool collide = (shape->shape_flags & 0x10) != 0;

	Tensor3D voxels(sizex, sizey, sizez);
	voxels.FromRunLengthEncoding(shape->voxels.palette_indexes);

	bool is_scaled = !CompareFloat(shape->scale, 0.1f);
	if (params.use_voxbox && voxels.IsFilledSingleColor() && !is_scaled) {
		uint8_t index = voxels.Get(0, 0, 0);
		Material palette_entry = palette.materials[index];

		entity_element->SetName("voxbox");
		WriteTransform(entity_element, shape->transform);
		xml.AddIntFloatAttribute(entity_element, "texture", shape->texture_tile, shape->texture_weight);
		xml.AddIntFloatAttribute(entity_element, "blendtexture", shape->blendtexture_tile, shape->blendtexture_weight);
		xml.AddFloatAttribute(entity_element, "density", shape->density, "1");
		xml.AddFloatAttribute(entity_element, "strength", shape->strength, "1");
		xml.AddBoolAttribute(entity_element, "collide", collide, true);
		//xml.AddBoolAttribute(entity_element, "prop", is_prop, false);
		xml.AddFloat3Attribute(entity_element, "size", sizex, sizey, sizez);
		xml.AddStrAttribute(entity_element, "material", MaterialKindName[palette_entry.kind], "none");
		xml.AddColorAttribute(entity_element, "color", palette_entry.rgba, "1 1 1");
		xml.AddFloat4Attribute(entity_element, "pbr", palette_entry.reflectivity, palette_entry.shinyness, palette_entry.metalness, palette_entry.emissive, "0 0 0 0");
		voxels.Clear();
		return;
	}

	Vector axis_offset(0.05f * (sizex - sizex % 2), 0.05f * (sizey - sizey % 2), 0);
	if (is_scaled) axis_offset = axis_offset * (10.0f * shape->scale);
	shape->transform.pos = shape->transform.pos + shape->transform.rot * axis_offset;
	shape->transform.rot = shape->transform.rot * QuatEuler(90, 0, 0);

	bool is_prop = false;
	if (shape->transform.isDefault()) {
		Entity* parent_entity = entity->parent;
		assert(parent_entity != NULL);
		assert(parent_entity->type == KindBody);
		Body* parent_body = static_cast<Body*>(parent_entity->self);
		if (parent_body->dynamic && parent_entity->children.getSize() == 1 && parent_entity->tags.getSize() == 0) {
			XMLElement* grandparent_element = parent_element->Parent()->ToElement();
			grandparent_element->DeleteChild(parent_element);
			parent_element = grandparent_element;
			WriteTransform(entity_element, parent_body->transform);
			is_prop = true;
		}
	}

	entity_element->SetName("vox");
	if (!is_prop) WriteTransform(entity_element, shape->transform);
	xml.AddIntFloatAttribute(entity_element, "texture", shape->texture_tile, shape->texture_weight);
	xml.AddIntFloatAttribute(entity_element, "blendtexture", shape->blendtexture_tile, shape->blendtexture_weight);
	xml.AddFloatAttribute(entity_element, "density", shape->density, "1");
	xml.AddFloatAttribute(entity_element, "strength", shape->strength, "1");
	xml.AddBoolAttribute(entity_element, "collide", collide, true);
	xml.AddBoolAttribute(entity_element, "prop", is_prop, false);

	if (volume > 0 && sizex <= 256 && sizey <= 256 && sizez <= 256) {
		string vox_folder = params.legacy_format ? "custom/" : "vox/";
		string vox_filename = params.map_folder + vox_folder + "palette" + to_string(shape->palette) + ".vox";
		string path_prefix = params.legacy_format ? "LEVEL/" : "MOD/vox/";
		string vox_path = path_prefix + "palette" + to_string(shape->palette) + ".vox";
		string vox_object = "shape" + to_string(handle);

		MV_FILE* vox_file;
		if (vox_files.find(shape->palette) == vox_files.end()) {
			vox_file = new MV_FILE(vox_filename.c_str());
			vox_files[shape->palette] = vox_file;
		} else
			vox_file = vox_files[shape->palette];

		MVShape mvshape = { vox_object.c_str(), 0, 0, sizez / 2, voxels };

		bool is_wheel_shape = false;
		if (params.remove_snow) {
			Entity* vox_parent = entity->parent;
			if (vox_parent != NULL && vox_parent->type == KindBody) {
				Entity* vox_grandparent = vox_parent->parent;
				if (vox_grandparent != NULL && vox_grandparent->type == KindWheel)
					is_wheel_shape = true;
			}
		}

		for (int z = 0; z < sizez; z++)
			for (int y = 0; y < sizey; y++)
				for (int x = 0; x < sizex; x++) {
					uint8_t index = voxels.Get(x, y, z);
					if (index != 0) {
						Material palette_entry = palette.materials[index];
						if (params.remove_snow && palette_entry.kind == MaterialKind::Unphysical &&
							int(255.0 * palette_entry.rgba.r) == 229 && int(255.0 * palette_entry.rgba.g) == 229 && int(255.0 * palette_entry.rgba.b) == 229)
							mvshape.voxels.Set(x, y, z, 0);

						vox_file->SetColor(index, 255.0 * palette_entry.rgba.r, 255.0 * palette_entry.rgba.g, 255.0 * palette_entry.rgba.b);
						vox_file->SetMaterial(index, palette_entry.kind, palette_entry.reflectivity, palette_entry.shinyness, palette_entry.metalness, palette_entry.emissive, palette_entry.rgba.a);
					}
				}

		if (params.remove_snow && !is_wheel_shape) {
			if (mvshape.voxels.Get(0, 0, 0) == 0)
				mvshape.voxels.Set(0, 0, 0, 255);
			if (mvshape.voxels.Get(sizex - 1, sizey - 1, sizez - 1) == 0)
				mvshape.voxels.Set(sizex - 1, sizey - 1, sizez - 1, 255);
			vox_file->SetColor(255, 255, 0, 0);
			vox_file->SetMaterial(255, 14, 0.1, 1.0, 0.0, 0.0, 1.0);
		}

		bool duplicated = vox_file->GetShapeName(mvshape, vox_object);
		if (!duplicated)
			vox_file->AddShape(mvshape);
		else
			voxels.Clear();

		xml.AddStrAttribute(entity_element, "file", vox_path);
		xml.AddStrAttribute(entity_element, "object", vox_object);
		xml.AddFloatAttribute(entity_element, "scale", 10.0 * shape->scale, "1");
	} else {
		string vox_folder = params.legacy_format ? "custom/" : "vox/";
		string vox_filename = params.map_folder + vox_folder + "palette" + to_string(shape->palette) + ".vox";
		string path_prefix = params.legacy_format ? "LEVEL/" : "MOD/vox/";
		string vox_path = path_prefix + "palette" + to_string(shape->palette) + ".vox";

		MV_FILE* vox_file;
		if (vox_files.find(shape->palette) == vox_files.end()) {
			vox_file = new MV_FILE(vox_filename.c_str());
			vox_files[shape->palette] = vox_file;
		} else
			vox_file = vox_files[shape->palette];

		entity_element->SetName("compound");
		for (int i = 0; i < (sizex + 256 - 1) / 256; i++)
			for (int j = 0; j < (sizey + 256 - 1) / 256; j++)
				for (int k = 0; k < (sizez + 256 - 1) / 256; k++)
					WriteCompound(handle, voxels, vox_file, vox_path, entity_element, shape, i, j, k);
		voxels.Clear();
	}
}

void WriteXML::WriteCompound(uint32_t handle, const Tensor3D &voxels, MV_FILE* compound_vox, string vox_file, XMLElement* compound_xml, Shape* shape, int i, int j, int k) {
	XMLElement* shape_xml = xml.CreateElement("vox");
	Palette palette = scene.palettes[shape->palette];

	int offsetx = 256 * i;
	int offsety = 256 * j;
	int offsetz = 256 * k;
	int sizex = shape->voxels.size[0];
	int sizey = shape->voxels.size[1];
	int sizez = shape->voxels.size[2];
	int part_sizex = min(256, sizex - offsetx);
	int part_sizey = min(256, sizey - offsety);
	int part_sizez = min(256, sizez - offsetz);

	float pos_x = 25.6 * i + (part_sizex / 2) * 0.1;
	float pos_y = 25.6 * k;
	float pos_z = -25.6 * j - (part_sizey / 2) * 0.1;
	pos_x -= (sizex / 2) * 0.1;
	pos_z += (sizey / 2) * 0.1;

	int mv_pos_x = -10 * pos_x;
	int mv_pos_y = 10 * pos_z;
	int mv_pos_z = 10 * pos_y + part_sizez / 2;

	string vox_object = "shape" + to_string(handle) + "_part" + to_string(i) + to_string(j) + to_string(k);
	MVShape mvshape = { vox_object, mv_pos_x, mv_pos_y, mv_pos_z, Tensor3D(part_sizex, part_sizey, part_sizez) };
	mvshape.voxels.Set(0, 0, 0, 255);
	mvshape.voxels.Set(part_sizex - 1, part_sizey - 1, part_sizez - 1, 255);
	compound_vox->SetColor(255, 255, 0, 0);
	compound_vox->SetMaterial(255, 14, 0.1, 1.0, 0.0, 0.0, 1.0);

	bool empty = true;
	for (int z = offsetz; z < part_sizez + offsetz; z++)
		for (int y = offsety; y < part_sizey + offsety; y++)
			for (int x = offsetx; x < part_sizex + offsetx; x++) {
				uint8_t index = voxels.Get(x, y, z);
				if (index != 0) {
					Material palette_entry = palette.materials[index];
					if (params.remove_snow && palette_entry.kind == MaterialKind::Unphysical &&
						int(255.0 * palette_entry.rgba.r) == 229 && int(255.0 * palette_entry.rgba.g) == 229 && int(255.0 * palette_entry.rgba.b) == 229)
						mvshape.voxels.Set(x - offsetx, y - offsety, z - offsetz, 0);
					else
						mvshape.voxels.Set(x - offsetx, y - offsety, z - offsetz, index);

					compound_vox->SetColor(index, 255.0 * palette_entry.rgba.r, 255.0 * palette_entry.rgba.g, 255.0 * palette_entry.rgba.b);
					compound_vox->SetMaterial(index, palette_entry.kind, palette_entry.reflectivity, palette_entry.shinyness, palette_entry.metalness, palette_entry.emissive, palette_entry.rgba.a);
					empty = false;
				}
			}
	if (!empty) {
		bool duplicated = compound_vox->GetShapeName(mvshape, vox_object);
		if (!duplicated)
			compound_vox->AddShape(mvshape);
		else
			mvshape.voxels.Clear();

		xml.AddFloat3Attribute(shape_xml, "pos", pos_x, pos_y, pos_z);
		xml.AddStrAttribute(shape_xml, "file", vox_file);
		xml.AddStrAttribute(shape_xml, "object", vox_object);
		xml.AddElement(compound_xml, shape_xml);
	}
}

static string ConcatTags(const Vec<Tag>& tags) {
	string tag_str = "";
	for (unsigned int i = 0; i < tags.getSize(); i++) {
		tag_str += tags[i].name;
		if (tags[i].value.length() > 0)
			tag_str += "=" + tags[i].value;
		if (i != tags.getSize() - 1)
			tag_str += " ";
	}
	return tag_str;
}

void WriteXML::WriteEntity(XMLElement* parent, Entity* entity) {
	XMLElement* entity_element = xml.CreateElement("unknown");
	xml.AddStrAttribute(entity_element, "tags", ConcatTags(entity->tags));
	xml.AddStrAttribute(entity_element, "desc", entity->desc);

	switch (entity->type) {
		case KindBody: {
			Body* body = static_cast<Body*>(entity->self);
			entity_element->SetName("body");

			Entity* body_parent = entity->parent;
			if (body_parent != NULL && body_parent->type == KindVehicle) {
				Vehicle* parent_vehicle = static_cast<Vehicle*>(body_parent->self);
				body->transform = TransformToLocalTransform(parent_vehicle->transform, body->transform);
			}

			// Change world body to group, and other static bodies
			if (!params.legacy_format && entity->parent == NULL && !body->dynamic && entity->tags.getSize() == 0) {
				entity_element->SetName("group");
				if (entity->handle == scene.world_body)
					xml.AddStrAttribute(entity_element, "name", "World Body");
				else
					xml.AddStrAttribute(entity_element, "name", "Static");
			} else if (parent == xml.GetScene())
				parent = xml.GetDynamicGroup();

			WriteTransform(entity_element, body->transform);
			xml.AddBoolAttribute(entity_element, "dynamic", body->dynamic, false);

			// If has parent, is static and no tags
			if (entity->parent != NULL && !body->dynamic && entity->tags.getSize() == 0)
				entity_element = NULL; // Ignore wheel body

			// If no children and no tags
			if (entity->children.getSize() == 0 && entity->tags.getSize() == 0)
				entity_element = NULL; // Ignore empty bodies with no tags
		}
			break;
		case KindShape:
			WriteShape(parent, entity_element, entity);
			break;
		case KindLight: {
			Light* light = static_cast<Light*>(entity->self);
			entity_element->SetName("light");

			Entity* light_parent = entity->parent;
			if (light_parent != NULL && light_parent->type == KindShape) {
				Shape* parent_shape = static_cast<Shape*>(light_parent->self);
				Transform light_tr = TransformToLocalTransform(parent_shape->transform, light->transform);
				WriteTransform(entity_element, light_tr);
			} else if (light_parent != NULL && light_parent->type == KindLocation) {
				Location* parent_location = static_cast<Location*>(light_parent->self);
				Transform light_tr = TransformToLocalTransform(parent_location->transform, light->transform);
				WriteTransform(entity_element, light_tr);
			} else if (light_parent != NULL && light_parent->type != KindScreen && light_parent->type != KindLight)
				WriteTransform(entity_element, light->transform);

			if (light->type == Capsule)
				xml.AddAttribute(entity_element, "type", "capsule");
			else if (light->type == Cone)
				xml.AddAttribute(entity_element, "type", "cone");
			else if (light->type == Area)
				xml.AddAttribute(entity_element, "type", "area");

			xml.AddFloat4Attribute(entity_element, "color", pow(light->color.r, 1 / 2.2f), pow(light->color.g, 1 / 2.2f), pow(light->color.b, 1 / 2.2f), light->color.a, "1 1 1 1");
			xml.AddFloatAttribute(entity_element, "scale", light->scale, "1");

			if (light->type == Cone) {
				xml.AddFloatAttribute(entity_element, "angle", 2.0 * deg(acos(light->angle)), "90");
				xml.AddFloatAttribute(entity_element, "penumbra", 2.0 * deg(acos(light->angle) - acos(light->penumbra)), "10");
			}

			if (light->type == Area)
				xml.AddFloat2Attribute(entity_element, "size", 2.0 * light->area_size[0], 2.0 * light->area_size[1]);
			else if (light->type == Capsule)
				xml.AddFloat2Attribute(entity_element, "size", 2.0 * light->capsule_size, light->size);
			else
				xml.AddFloatAttribute(entity_element, "size", light->size, "0.1");

			xml.AddFloatAttribute(entity_element, "reach", light->reach);
			xml.AddFloatAttribute(entity_element, "unshadowed", light->unshadowed, "0");
			xml.AddFloatAttribute(entity_element, "fogscale", light->fogscale, "1");
			xml.AddFloatAttribute(entity_element, "fogiter", light->fogiter, "1");
			xml.AddStrFloatAttribute(entity_element, "sound",  light->sound.path, light->sound.volume);
			xml.AddFloatAttribute(entity_element, "glare", light->glare, "0");

			if (entity->parent == NULL || entity->handle == scene.flashlight)
				entity_element = NULL; // Ignore player flashlight
		}
			break;
		case KindLocation: {
			Location* location = static_cast<Location*>(entity->self);
			entity_element->SetName("location");

			Entity* location_parent = entity->parent;
			while (location_parent != NULL && location_parent->type != KindVehicle)
				location_parent = location_parent->parent;
			if (location_parent != NULL) {
				// Location is inside a vehicle
				entity_element = NULL;
				break;
			}

			location_parent = entity->parent;
			if (location_parent != NULL && location_parent->type == KindShape) {
				// The location is inside a static shape
				Shape* parent_shape = static_cast<Shape*>(location_parent->self);
				Transform loc_tr = TransformToLocalTransform(parent_shape->transform, location->transform);
				WriteTransform(entity_element, loc_tr);

				// The location is inside a dynamic body
				Entity* location_grandparent = location_parent->parent;
				if (location_grandparent != NULL && location_grandparent->type == KindBody) {
					Body* grandparent_body = static_cast<Body*>(location_grandparent->self);
					if (grandparent_body->dynamic) {
						Transform location_tr = TransformToLocalTransform(grandparent_body->transform, location->transform);
						location_tr = TransformToLocalTransform(parent_shape->transform, location_tr);
						WriteTransform(entity_element, location_tr);
					}
				}
			} else if (location_parent != NULL && location_parent->type == KindBody) {
				Body* parent_body = static_cast<Body*>(location_parent->self);
				Transform loc_tr = TransformToLocalTransform(parent_body->transform, location->transform);
				WriteTransform(entity_element, loc_tr);
			} else if (location_parent != NULL && location_parent->type == KindTrigger) {
				Trigger* parent_trigger = static_cast<Trigger*>(location_parent->self);
				Transform loc_tr = TransformToLocalTransform(parent_trigger->transform, location->transform);
				WriteTransform(entity_element, loc_tr);
			} else
				WriteTransform(entity_element, location->transform);

			if (parent == xml.GetScene())
				parent = xml.GetLocationsGroup();
		}
			break;
		case KindWater: {
			Water* water = static_cast<Water*>(entity->self);
			entity_element->SetName("water");
			WriteTransform(entity_element, water->transform);
			xml.AddStrAttribute(entity_element, "type", "polygon");
			xml.AddFloatAttribute(entity_element, "depth", water->depth, "10");
			xml.AddFloatAttribute(entity_element, "wave", water->wave, "0.5");
			xml.AddFloatAttribute(entity_element, "ripple", water->ripple, "0.5");
			xml.AddFloatAttribute(entity_element, "motion", water->motion, "0.5");
			xml.AddFloatAttribute(entity_element, "foam", water->foam, "0.5");
			xml.AddColorAttribute(entity_element, "color", water->color, "0.01 0.01 0.01");
			xml.AddFloatAttribute(entity_element, "visibility", water->visibility, "3");
			WriteVertices(entity_element, water->water_vertices);

			if (parent == xml.GetScene())
				parent = xml.GetWaterGroup();
		}
			break;
		case KindJoint: {
			Joint* joint = static_cast<Joint*>(entity->self);
			if (joint->type == _Rope) {
				entity_element->SetName("rope");
				xml.AddFloatAttribute(entity_element, "size", joint->size, "0.2");
				xml.AddColorAttribute(entity_element, "color", joint->rope->color, "0 0 0");

				int knot_count = joint->rope->segments.getSize();
				if (knot_count > 0) {
					XMLElement* location_from = xml.CreateElement("location");
					Vector rope_start = joint->rope->segments[0].from;
					Vector rope_end = joint->rope->segments[knot_count - 1].to;

					xml.AddElement(entity_element, location_from);
					xml.AddVectorAttribute(location_from, "pos", rope_start);

					XMLElement* location_to = xml.CreateElement("location");
					xml.AddElement(entity_element, location_to);
					xml.AddVectorAttribute(location_to, "pos", rope_end);

					Vector rope_dir = rope_end - rope_start;
					float rope_length = rope_dir.length();
					float slack = joint->rope->slack - rope_length;
					xml.AddFloatAttribute(entity_element, "slack", slack, "0");
				}

				xml.AddFloatAttribute(entity_element, "strength", joint->rope->strength, "1");
				xml.AddFloatAttribute(entity_element, "maxstretch", joint->rope->maxstretch, "0");

				if (parent == xml.GetScene())
					parent = xml.GetRopesGroup();
			} else
				entity_element = NULL; // Process joints on a second pass
		}
			break;
		case KindVehicle: {
			Vehicle* vehicle = static_cast<Vehicle*>(entity->self);
			entity_element->SetName("vehicle");
			WriteTransform(entity_element, vehicle->transform);

			xml.AddStrFloatAttribute(entity_element, "sound", vehicle->properties.sound.name, vehicle->properties.sound.pitch, "medium");
			xml.AddFloatAttribute(entity_element, "spring", vehicle->properties.spring, "1");
			xml.AddFloatAttribute(entity_element, "damping", vehicle->properties.damping, "1");
			xml.AddFloatAttribute(entity_element, "topspeed", 3.6 * vehicle->properties.topspeed, "70");
			xml.AddFloatAttribute(entity_element, "acceleration", vehicle->properties.acceleration, "1");
			xml.AddFloatAttribute(entity_element, "strength", vehicle->properties.strength, "1");
			xml.AddFloatAttribute(entity_element, "antispin", vehicle->properties.antispin, "0");
			xml.AddFloatAttribute(entity_element, "antiroll", vehicle->properties.antiroll, "0");
			xml.AddFloatAttribute(entity_element, "difflock", vehicle->difflock, "0");
			xml.AddFloatAttribute(entity_element, "steerassist", vehicle->properties.steerassist, "0");
			xml.AddFloatAttribute(entity_element, "friction", vehicle->properties.friction, "1.3");

			int exhausts_count = vehicle->exhausts.getSize();
			for (int i = 0; i < exhausts_count; i++) {
				string exhaust_tag = "exhaust";
				if (vehicle->exhausts[i].strength != 1.0f)
					exhaust_tag += "=" + FloatToString(vehicle->exhausts[i].strength);

				XMLElement* exhaust = xml.CreateElement("location");
				xml.AddElement(entity_element, exhaust);
				xml.AddStrAttribute(exhaust, "tags", exhaust_tag);
				WriteTransform(exhaust, vehicle->exhausts[i].transform);
			}

			XMLElement* player = xml.CreateElement("location");
			xml.AddElement(entity_element, player);
			xml.AddAttribute(player, "tags", "player");
			xml.AddVectorAttribute(player, "pos", vehicle->player);

			if (!vehicle->camera.isZero()) {
				XMLElement* camera = xml.CreateElement("location");
				xml.AddElement(entity_element, camera);
				xml.AddAttribute(camera, "tags", "camera");
				xml.AddVectorAttribute(camera, "pos", vehicle->camera);
			}

			if (!vehicle->exit.isZero()) {
				XMLElement* exit = xml.CreateElement("location");
				xml.AddElement(entity_element, exit);
				xml.AddAttribute(exit, "tags", "exit");
				xml.AddVectorAttribute(exit, "pos", vehicle->exit);
			}

			bool is_boat = false;
			for (unsigned int i = 0; i < entity->tags.getSize() && !is_boat; i++)
				is_boat = entity->tags[i].name == "boat";

			if (is_boat) {
				XMLElement* propeller = xml.CreateElement("location");
				xml.AddElement(entity_element, propeller);
				xml.AddAttribute(propeller, "tags", "propeller");
				xml.AddVectorAttribute(propeller, "pos", vehicle->propeller);
			}

			if (parent == xml.GetScene())
				parent = xml.GetVehiclesGroup();
		}
			break;
		case KindWheel: {
			Wheel* wheel = static_cast<Wheel*>(entity->self);
			entity_element->SetName("wheel");

			Entity* wheel_parent = entity->parent;
			if (wheel_parent != NULL && wheel_parent->type == KindShape) {
				Shape* parent_shape = static_cast<Shape*>(wheel_parent->self);
				Transform wheel_tr = TransformToLocalTransform(parent_shape->transform, wheel->transform);
				WriteTransform(entity_element, wheel_tr);
			} else
				WriteTransform(entity_element, wheel->transform);

			xml.AddFloatAttribute(entity_element, "drive", wheel->drive, "0");
			xml.AddFloatAttribute(entity_element, "steer", wheel->steer, "0");
			xml.AddFloatNAttribute(entity_element, "travel", wheel->travel, 2, "-0.1 0.1");
		}
			break;
		case KindScreen: {
			Screen* screen = static_cast<Screen*>(entity->self);
			entity_element->SetName("screen");

			Entity* screen_parent = entity->parent;
			if (screen_parent != NULL && screen_parent->type == KindShape) {
				Shape* parent_shape = static_cast<Shape*>(screen_parent->self);
				// Revert applied transform
				Transform parent_shape_tr = TransformToLocalTransform(parent_shape->old_transform, parent_shape->transform);
				Transform screen_tr = TransformToLocalTransform(parent_shape_tr, screen->transform);
				WriteTransform(entity_element, screen_tr);
			} else
				WriteTransform(entity_element, screen->transform);

			xml.AddFloatNAttribute(entity_element, "size", screen->size, 2, "0.9 0.5");
			xml.AddFloatAttribute(entity_element, "bulge", screen->bulge, "0.08");

			string resolution = to_string(screen->resolution[0]) + " " + to_string(screen->resolution[1]);
			xml.AddStrAttribute(entity_element, "resolution", resolution, "640 480");

			string script_file = screen->script;
			if (!params.level_id.empty()) {
				string prefix = "data/level/" + params.level_id;
				if (script_file.find(prefix) == 0)
					script_file = "LEVEL" + script_file.substr(prefix.size());
			}
			string prefix = "data/";
			if (script_file.find(prefix) == 0)
				script_file = script_file.substr(prefix.size());

			xml.AddStrAttribute(entity_element, "script", script_file);
			xml.AddBoolAttribute(entity_element, "enabled", screen->enabled, false);
			xml.AddBoolAttribute(entity_element, "interactive", screen->interactive, false);
			xml.AddFloatAttribute(entity_element, "emissive", screen->emissive, "1");
			xml.AddFloatAttribute(entity_element, "fxraster", screen->fxraster, "0");
			xml.AddFloatAttribute(entity_element, "fxca", screen->fxca, "0");
			xml.AddFloatAttribute(entity_element, "fxnoise", screen->fxnoise, "0");
			xml.AddFloatAttribute(entity_element, "fxglitch", screen->fxglitch, "0");
		}
			break;
		case KindTrigger: {
			Trigger* trigger = static_cast<Trigger*>(entity->self);
			entity_element->SetName("trigger");
			if (trigger->type == TrBox) {
				Vector offset = Vector(0, trigger->box_size[1], 0);
				trigger->transform.pos = trigger->transform.pos - trigger->transform.rot * offset;
			}
			WriteTransform(entity_element, trigger->transform);

			if (trigger->type == TrSphere)
				xml.AddFloatAttribute(entity_element, "size", trigger->sphere_size, "10");
			else if (trigger->type == TrBox) {
				xml.AddAttribute(entity_element, "type", "box");
				xml.AddFloat3Attribute(entity_element, "size", 2.0 * trigger->box_size[0], 2.0 * trigger->box_size[1], 2.0 * trigger->box_size[2]);
			} else if (trigger->type == TrPolygon) {
				xml.AddAttribute(entity_element, "type", "polygon");
				xml.AddFloatAttribute(entity_element, "size", trigger->polygon_size, "10");
				WriteVertices(entity_element, trigger->polygon_vertices);
			}
			xml.AddStrFloatAttribute(entity_element, "sound", trigger->sound.path, trigger->sound.volume);
			xml.AddFloatAttribute(entity_element, "soundramp", trigger->sound.soundramp, "2");

			if (parent == xml.GetScene())
				parent = xml.GetTriggersGroup();
		}
			break;
		case KindScript:
			entity_element = NULL; // Process scripts on a second pass
			break;
	}

	if (entity_element != NULL)
		xml.AddElement(parent, entity_element, entity->handle);
	else
		entity_element = parent;

	for (unsigned int i = 0; i < entity->children.getSize(); i++)
		WriteEntity(entity_element, entity->children[i]);
}

void WriteXML::WriteEntity2ndPass(Entity* entity) {
	if (entity->type == KindVehicle) {
		Vehicle* vehicle = static_cast<Vehicle*>(entity->self);
		int vital_count = vehicle->vitals.getSize();
		for (int i = 0; i < vital_count; i++) {
			uint32_t body_handle = vehicle->vitals[i].body_handle;
			XMLElement* xml_body = xml.GetNode(body_handle);
			if (xml_body != NULL) {
				XMLElement* vital = xml.CreateElement("location");
				xml.AddElement(xml_body, vital);
				xml.AddAttribute(vital, "tags", "vital");
				xml.AddVectorAttribute(vital, "pos", vehicle->vitals[i].position);
			}
		}
	} else if (entity->type == KindJoint) {
		Joint* joint = static_cast<Joint*>(entity->self);
		if (joint->type != _Rope) {
			XMLElement* entity_element = xml.CreateElement("joint");
			xml.AddStrAttribute(entity_element, "tags", ConcatTags(entity->tags));

			uint32_t shape_handle = joint->shapes[0];
			if (shape_handle == 0) return;
			assert(entity_mapping.find(shape_handle) != entity_mapping.end());
			Entity* parent_entity = entity_mapping[shape_handle];
			assert(parent_entity != NULL);
			assert(parent_entity->type == KindShape);
			Shape* shape = static_cast<Shape*>(parent_entity->self);
			assert(parent_entity->parent != NULL);
			assert(parent_entity->parent->type == KindBody);
			XMLElement* parent_element = xml.GetNode(shape_handle);
			assert(parent_element != NULL);

			Vector relative_pos = joint->positions[0];
			Quat relative_rot;
			if (joint->type != Ball) {
				Vector joint_axis(joint->axis[0]);
				if (joint_axis == Vector(1, 0, 0))
					relative_rot = QuatEuler(0, 90, 0);
				else if (joint_axis == Vector(-1, 0, 0))
					relative_rot = QuatEuler(0, -90, 0);
				else if (joint_axis == Vector(0, 1, 0))
					relative_rot = QuatEuler(-90, 0, 0);
				else if (joint_axis == Vector(0, -1, 0))
					relative_rot = QuatEuler(90, 0, 0);
				else if (joint_axis == Vector(0, 0, 1))
					relative_rot = Quat();
				else if (joint_axis == Vector(0, 0, -1))
					relative_rot = QuatEuler(0, 180, 0);
				else {
					double a = -asin(joint_axis.y);
					double b = asin(joint_axis.x / cos(a));
					//double b2 = acos(joint_axis.z / cos(a));
					if (!isnan(a) && !isnan(b))
						relative_rot = QuatEulerRad(a, b, 0);
					else {
						relative_rot = Quat();
						xml.AddStrAttribute(entity_element, "name", "FIXROT");
					}
				}
			}
			Transform joint_tr = Transform(relative_pos, relative_rot);
			if (parent_element->Attribute("prop") == NULL)
				joint_tr = TransformToLocalTransform(shape->transform, joint_tr);
			WriteTransform(entity_element, joint_tr);

			if (joint->type == Hinge)
				xml.AddAttribute(entity_element, "type", "hinge");
			else if (joint->type == Prismatic)
				xml.AddAttribute(entity_element, "type", "prismatic");

			xml.AddFloatAttribute(entity_element, "size", joint->size, "0.1");
			if (joint->type != Prismatic) {
				xml.AddFloatAttribute(entity_element, "rotstrength", joint->rotstrength, "0");
				xml.AddFloatAttribute(entity_element, "rotspring", joint->rotspring, "0.5");
			}
			xml.AddBoolAttribute(entity_element, "collide", joint->collide, false);
			if (joint->type == Hinge) {
				if (joint->limits[0] != 0.0 || joint->limits[1] != 0.0)
					xml.AddFloat2Attribute(entity_element, "limits", deg(joint->limits[0]), deg(joint->limits[1]));
			} else if (joint->type == Prismatic)
				xml.AddFloatNAttribute(entity_element, "limits", joint->limits, 2, "0 0");
			xml.AddBoolAttribute(entity_element, "sound", joint->sound, false);
			if (joint->type == Prismatic)
				xml.AddBoolAttribute(entity_element, "autodisable", joint->autodisable, false);
			if (parent_element != NULL)
				xml.AddElement(parent_element, entity_element, entity->handle);
		}
	} else if (entity->type == KindScript) {
		Script* script = static_cast<Script*>(entity->self);
		XMLElement* entity_element = xml.CreateElement("script");

		string script_file = script->file;
		string prefix = "data/script/";
		if (script_file.find(prefix) == 0)
			script_file = script_file.substr(prefix.size());

		if (!params.level_id.empty()) {
			prefix = "data/level/" + params.level_id + "/";
			if (script_file.find(prefix) == 0)
				script_file = "LEVEL/" + script_file.substr(prefix.size());
		}

		if (!params.dlc_id.empty()) {
			prefix = "dlcs/" + params.dlc_id + "/";
			if (script_file.find(prefix) == 0)
				script_file = "MOD/" + script_file.substr(prefix.size());
		}

		prefix = "RAW:";
		if (script_file.find(prefix) == 0)
			return;

		if (script_file == "achievements.lua" || script_file == "creativemode.lua" || script_file == "explosion.lua" || script_file == "fx.lua" || script_file == "spawn.lua")
			return;

		xml.AddElement(xml.GetScriptsGroup(), entity_element);

		xml.AddStrAttribute(entity_element, "file", script_file);
		assert(script->params.getSize() <= 4);
		for (unsigned int i = 0; i < script->params.getSize(); i++) {
			string param_index = "param";
			param_index += to_string(i);
			string param = script->params[i].key;
			if (script->params[i].value.length() > 0)
				param += "=" + script->params[i].value;
			xml.AddStrAttribute(entity_element, param_index.c_str(), param);
		}

		for (unsigned int j = 0; j < script->entity_handles.getSize(); j++) {
			uint32_t entity_handle = script->entity_handles[j];
			XMLElement* entity_child = xml.GetNode(entity_handle);
			if (entity_child != NULL && strcmp(entity_child->Name(), "joint") != 0) {
				while (entity_child->Parent() != NULL && entity_child->Parent()->ToElement() != xml.GetScene() &&
					strcmp(entity_child->Parent()->ToElement()->Name(), "group") != 0) {
					entity_child = entity_child->Parent()->ToElement();
				}
				if (entity_element != entity_child)
					xml.MoveElement(entity_element, entity_child);
			}
		}
	}

	for (int i = entity->children.getSize() - 1; i >= 0; i--)
		WriteEntity2ndPass(entity->children[i]);
}
