#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "scene.h"
#include "entity.h"
#include "math_utils.h"
#include "misc_utils.h"
#include "vox_writer.h"
#include "xml_writer.h"
#include "write_scene.h"
#include "../lib/tinyxml2.h"

static MV_Color ToMV(const Color& color) {
	uint8_t r = 255.0 * color.r;
	uint8_t g = 255.0 * color.g;
	uint8_t b = 255.0 * color.b;
	return { r, g, b, 255 };
}

static MV_Material ToMV(const Material& material) {
	MV_Material mat;
	mat.td_type = material.type;
	const Color& color = material.rgba;
	if (color.a < 1.0) {
		mat.type = GLASS;
		mat.properties.glass.roughness = 1.0 - material.shinyness;
	} else if (material.emissive > 0) {
		mat.type = EMIT;
		int flux = 0;
		if (material.emissive > 100.0)
			flux = 4;
		else if (material.emissive > 10.0)
			flux = 3;
		else if (material.emissive > 1.0)
			flux = 2;
		else if (material.emissive > 0.1)
			flux = 1;
		mat.properties.emit.emission = material.emissive / pow(10, flux - 1);
		mat.properties.emit.power = flux;
	} else if (material.reflectivity > 0 || material.shinyness > 0 || material.metalness > 0) {
		mat.type = METAL;
		mat.properties.metal.roughness = 1.0 - material.shinyness;
		mat.properties.metal.specular = 1.0 + material.reflectivity;
		mat.properties.metal.metallic = material.metalness;
	} else
		mat.type = DIFFUSE;
	return mat;
}

static string ConcatTags(const SmallVec<Tag>& tags) {
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

WriteXML::WriteXML(ConverterParams params) : params(params) {
	InitScene(params.bin_path.c_str());
}

WriteXML::~WriteXML() {
	for (map<uint32_t, MV_FILE*>::iterator it = vox_files.begin(); it != vox_files.end(); it++)
		delete it->second;
	vox_files.clear();
}

void WriteXML::WriteScene() {
	string version_str = to_string(scene.version[0]) + "." + to_string(scene.version[1]) + "." + to_string(scene.version[2]);
	xml.AddStringAttribute(xml.GetScene(), "version", version_str);
	xml.AddVec3Attribute(xml.GetScene(), "shadowVolume", scene.shadow_volume, "100 25 100");
}

void WriteXML::WriteSpawnpoint() {
	XMLElement* spawnpoint = xml.AddChildElement(xml.GetScene(), "spawnpoint");
	scene.spawnpoint.rot = scene.spawnpoint.rot * QuatEuler(0, 180, 0);
	xml.AddTransformAttribute(spawnpoint, scene.spawnpoint);
}

void WriteXML::WriteEnvironment() {
	Skybox* skybox = &scene.environment.skybox;
	Fog* fog = &scene.environment.fog;
	Environment::Water* water = &scene.environment.water;
	Snow* snow = &scene.environment.snow;

	XMLElement* environment = xml.AddChildElement(xml.GetScene(), "environment");

	string skybox_texture = skybox->texture;
	string prefix = "data/env/";
	if (skybox_texture.find(prefix) == 0)
		skybox_texture = skybox_texture.substr(prefix.size());

	if (!params.dlc_id.empty()) {
		prefix = "dlcs/" + params.dlc_id + "/env/";
		if (skybox_texture.find(prefix) == 0)
			skybox_texture = skybox_texture.substr(prefix.size());
	}

	if (fog->type == Fog::Classic)
		fog->params.y += fog->params.x; // end = start + distance

	xml.AddStringAttribute(environment, "skybox", skybox_texture, "cloudy.dds");
	xml.AddColorAttribute(environment, "skyboxtint", skybox->tint, "1 1 1");
	xml.AddFloatAttribute(environment, "skyboxbrightness", skybox->brightness, "1");
	xml.AddFloatAttribute(environment, "skyboxrot", deg(skybox->rot), "0");
	xml.AddVec3Attribute(environment, "skyboxaxis", Vec3(0, 1, 0), "0 1 0");
	xml.AddStringAttribute(environment, "lensdirt", scene.environment.lensdirt);
	xml.AddColorAttribute(environment, "constant", skybox->constant, "0.003 0.003 0.003");
	xml.AddFloatAttribute(environment, "ambient", skybox->ambient, "1");
	xml.AddFloatAttribute(environment, "ambientexponent", skybox->ambientexponent, "1.3");
	xml.AddStringAttribute(environment, "fog", FogType[fog->type], "classic");
	xml.AddColorAttribute(environment, "fogColor", fog->color, "1 1 1");
	xml.AddVec4Attribute(environment, "fogParams", fog->params, "40 100 0.9 4");
	xml.AddFloatAttribute(environment, "fogHeightOffset", fog->height_offset, "0");
	xml.AddFloatAttribute(environment, "sunBrightness", skybox->sun.brightness, "0");
	xml.AddColorAttribute(environment, "sunColorTint", skybox->sun.colortint, "1 1 1");
	if (!skybox->sun.auto_dir)
		xml.AddVec3Attribute(environment, "sunDir", skybox->sun.dir, "auto");
	xml.AddFloatAttribute(environment, "sunSpread", skybox->sun.spread, "0");
	xml.AddFloatAttribute(environment, "sunLength", skybox->sun.length, "32");
	xml.AddFloatAttribute(environment, "sunFogScale", skybox->sun.fogscale, "1");
	xml.AddFloatAttribute(environment, "sunGlare", skybox->sun.glare, "1");
	xml.AddVec2Attribute(environment, "exposure", scene.environment.exposure, "0 10");
	xml.AddFloatAttribute(environment, "brightness", scene.environment.brightness, "1");
	xml.AddFloatAttribute(environment, "wetness", water->wetness, "0");
	xml.AddFloatAttribute(environment, "puddleamount", water->puddleamount, "0");
	xml.AddFloatAttribute(environment, "puddlesize", 0.01 / water->puddlesize, "0.5");
	xml.AddFloatAttribute(environment, "rain", water->rain, "0");
	xml.AddBoolAttribute(environment, "nightlight", scene.environment.nightlight, true);
	xml.AddSoundAttribute(environment, "ambience", scene.environment.ambience, "outdoor/field.ogg");
	xml.AddFloatAttribute(environment, "fogscale", scene.environment.fogscale, "1");
	xml.AddFloatAttribute(environment, "slippery", scene.environment.slippery, "0");
	xml.AddFloatAttribute(environment, "waterhurt", scene.environment.waterhurt, "0");
	xml.AddVec4Attribute(environment, "snowdir", snow->dir, "0 -1 0 0.2");
	xml.AddVec2Attribute(environment, "snowamount", snow->amount, "0 0");
	xml.AddBoolAttribute(environment, "snowonground", snow->onground && !params.remove_snow, false);
	xml.AddVec3Attribute(environment, "wind", scene.environment.wind, "0 0 0");

	params.remove_snow = params.remove_snow && snow->onground; // Only remove snow if there is snow to remove
}

void WriteXML::WriteBoundary() {
	int vertex_count = scene.boundary.vertices.getSize();
	if (vertex_count == 0)
		return;

	XMLElement* boundary = xml.AddChildElement(xml.GetScene(), "boundary");
	xml.AddFloatAttribute(boundary, "padleft", -scene.boundary.padleft, "5");
	xml.AddFloatAttribute(boundary, "padright", scene.boundary.padright, "5");
	xml.AddFloatAttribute(boundary, "padtop", -scene.boundary.padtop, "5");
	xml.AddFloatAttribute(boundary, "padbottom", scene.boundary.padbottom, "5");
	xml.AddFloatAttribute(boundary, "maxheight", scene.boundary.maxheight, "0");
	xml.AddVerticesAttribute(boundary, scene.boundary.vertices);
}

void WriteXML::WritePostProcessing() {
	XMLElement* postprocessing = xml.AddChildElement(xml.GetScene(), "postprocessing");
	xml.AddFloatAttribute(postprocessing, "saturation", scene.postpro.saturation, "1");
	xml.AddColorAttribute(postprocessing, "colorbalance", scene.postpro.colorbalance, "1 1 1");
	xml.AddFloatAttribute(postprocessing, "brightness", scene.postpro.brightness, "1");
	xml.AddFloatAttribute(postprocessing, "gamma", scene.postpro.gamma, "1");
	xml.AddFloatAttribute(postprocessing, "bloom", scene.postpro.bloom, "1");
}

void WriteXML::SaveXML() {
	string main_xml_path = params.map_folder + (params.legacy_format ? "custom.xml" : "main.xml");
	xml.SaveFile(main_xml_path.c_str());
}

void WriteXML::SaveVoxFiles() {
	for (map<uint32_t, MV_FILE*>::iterator it = vox_files.begin(); it != vox_files.end(); it++)
		it->second->SaveModel(params.compress_vox);
}

void WriteXML::WriteEntities() {
	xml.CreateGroups();

	for (unsigned int i = 0; i < scene.entities.getSize(); i++)
		WriteEntity(xml.GetScene(), scene.entities[i]);

	for (unsigned int i = 0; i < scene.entities.getSize(); i++)
		WriteEntity2ndPass(scene.entities[i]);

	if (scene.driven_vehicle != 0) {
		XMLElement* vehicle_xml = xml.GetEntityElement(scene.driven_vehicle);
		if (vehicle_xml != nullptr)
			xml.AddBoolAttribute(vehicle_xml, "driven", true, false);
	}
}

static Transform GetEntityTransform(const Entity* entity) {
	if (entity == nullptr)
		return Transform();
	switch (entity->type) {
	case Entity::Body: {
		Body* body = static_cast<Body*>(entity->self);
		return body->transform;
	}
	case Entity::Shape: {
		Shape* shape = static_cast<Shape*>(entity->self);
		return shape->transform;
	}
	case Entity::Light: {
		Light* light = static_cast<Light*>(entity->self);
		return light->transform;
	}
	case Entity::Location: {
		Location* location = static_cast<Location*>(entity->self);
		return location->transform;
	}
	case Entity::Water: {
		Water* water = static_cast<Water*>(entity->self);
		return water->transform;
	}
	case Entity::Vehicle: {
		Vehicle* vehicle = static_cast<Vehicle*>(entity->self);
		return vehicle->transform;
	}
	case Entity::Wheel: {
		Wheel* wheel = static_cast<Wheel*>(entity->self);
		return wheel->transform;
	}
	case Entity::Screen: {
		Screen* screen = static_cast<Screen*>(entity->self);
		return screen->transform;
	}
	case Entity::Trigger: {
		Trigger* trigger = static_cast<Trigger*>(entity->self);
		return trigger->transform;
	}
	case Entity::Animator: {
		Animator* animator = static_cast<Animator*>(entity->self);
		return animator->transform;
	}
	default:
		return Transform();
	}
}

static Transform GetLocalTransform(const Entity* parent, Transform tr) {
	Transform parent_tr = GetEntityTransform(parent);
	return TransformToLocalTransform(parent_tr, tr);
}

void WriteXML::WriteBody(XMLElement* element, const Body* body, const Entity* parent) {
	element->SetName("body");
	xml.AddTransformAttribute(element, GetLocalTransform(parent, body->transform));
	xml.AddBoolAttribute(element, "dynamic", body->dynamic, false);
	if (element->Parent() == xml.GetScene() && body->dynamic)
		xml.GetGroupElement(PROP)->InsertEndChild(element);
}

void WriteXML::WriteShape(XMLElement* element, Shape* shape, uint32_t handle) {
	int sizex = shape->voxels.sizex;
	int sizey = shape->voxels.sizey;
	int sizez = shape->voxels.sizez;
	shape->original_tr = shape->transform;
	bool is_scaled = !CompareFloat(shape->voxels.scale, 0.1f);
	if (params.use_voxbox && !is_scaled && shape->decoded_voxels.IsFilledSingleColor())
		WriteVoxbox(element, shape);
	else if (sizex <= 256 && sizey <= 256 && sizez <= 256)
		WriteVox(element, shape, handle);
	else
		WriteCompound(element, shape, handle);
}

void WriteXML::WriteVox(XMLElement* element, Shape* shape, uint32_t handle) {
	int sizex = shape->voxels.sizex;
	int sizey = shape->voxels.sizey;
	int sizez = shape->voxels.sizez;

	Vec3 axis_offset(0.05f * (sizex - sizex % 2), 0.05f * (sizey - sizey % 2), 0);
	axis_offset = axis_offset * (10.0f * shape->voxels.scale);
	Transform shape_transform = shape->transform;
	shape_transform.pos = shape_transform.pos + shape_transform.rot * axis_offset;
	shape_transform.rot = shape_transform.rot * QuatEuler(90, 0, 0);
	shape->transform = shape_transform;

	string vox_folder = params.legacy_format ? "custom/" : "vox/";
	string vox_filename ="palette" + to_string(shape->voxels.palette_id) + ".vox";
	string vox_full_path = params.map_folder + vox_folder + vox_filename;
	string path_prefix = params.legacy_format ? "LEVEL/" : "MOD/vox/";
	string vox_path = path_prefix + vox_filename;
	string vox_object = "shape" + to_string(handle);

	MV_FILE* vox_file;
	if (vox_files.find(shape->voxels.palette_id) == vox_files.end()) {
		vox_file = new MV_FILE(vox_full_path);
		vox_files[shape->voxels.palette_id] = vox_file;
	} else
		vox_file = vox_files[shape->voxels.palette_id];

	MV_Shape mvshape = { vox_object, 0, 0, sizez / 2, shape->decoded_voxels };
	// Add voxels in opposite corners to prevent shape from changing size when removing snow
	if (params.remove_snow) {
		if (mvshape.voxels.Get(0, 0, 0) != 0)
			mvshape.voxels.Set(0, 0, 0, 255);
		if (mvshape.voxels.Get(sizex - 1, sizey - 1, sizez - 1) != 0)
			mvshape.voxels.Set(sizex - 1, sizey - 1, sizez - 1, 255);
		vox_file->SetEntry(255, HOLE_COLOR, HOLE_MATERIAL);
	}

	const Palette& palette = scene.palettes[shape->voxels.palette_id];
	for (int z = 0; z < sizez; z++)
		for (int y = 0; y < sizey; y++)
			for (int x = 0; x < sizex; x++) {
				uint8_t index = shape->decoded_voxels.Get(x, y, z);
				if (index != 0) {
					// Remove snow voxels
					if (params.remove_snow && index == 254)
						mvshape.voxels.Set(x, y, z, 0);
					// Add used palette entries
					const Material& palette_entry = palette.materials[index];
					vox_file->SetEntry(index, ToMV(palette_entry.rgba), ToMV(palette_entry));
				}
			}

	bool duplicated = vox_file->GetShapeName(mvshape, vox_object);
	if (!duplicated)
		vox_file->AddShape(mvshape);

	bool collide = (shape->shape_flags & 0x10) != 0;

	element->SetName("vox");
	xml.AddTransformAttribute(element, shape_transform);
	xml.AddTextureAttribute(element, "texture", shape->texture);
	xml.AddTextureAttribute(element, "blendtexture", shape->blendtexture);
	xml.AddFloatAttribute(element, "density", shape->density, "1");
	xml.AddFloatAttribute(element, "strength", shape->strength, "1");
	xml.AddBoolAttribute(element, "collide", collide, true);

	xml.AddStringAttribute(element, "file", vox_path);
	xml.AddStringAttribute(element, "object", vox_object);
	xml.AddFloatAttribute(element, "scale", 10.0 * shape->voxels.scale, "1");
}

void WriteXML::WriteVoxbox(XMLElement* element, const Shape* shape) {
	int sizex = shape->voxels.sizex;
	int sizey = shape->voxels.sizey;
	int sizez = shape->voxels.sizez;

	bool collide = (shape->shape_flags & 0x10) != 0;
	uint8_t index = shape->decoded_voxels.Get(0, 0, 0);
	const Palette& palette = scene.palettes[shape->voxels.palette_id];
	const Material& palette_entry = palette.materials[index];

	element->SetName("voxbox");
	xml.AddTransformAttribute(element, shape->transform);
	xml.AddTextureAttribute(element, "texture", shape->texture);
	xml.AddTextureAttribute(element, "blendtexture", shape->blendtexture);
	xml.AddFloatAttribute(element, "density", shape->density, "1");
	xml.AddFloatAttribute(element, "strength", shape->strength, "1");
	xml.AddBoolAttribute(element, "collide", collide, true);
	xml.AddVec3Attribute(element, "size", Vec3(sizex, sizey, sizez), "50 30 20");
	xml.AddStringAttribute(element, "material", MaterialName[palette_entry.type], "none");
	xml.AddColorAttribute(element, "color", palette_entry.rgba, "1 1 1");
	xml.AddVec4Attribute(element, "pbr", Vec4(palette_entry.reflectivity, palette_entry.shinyness, palette_entry.metalness, palette_entry.emissive), "0 0 0 0");
}

void WriteXML::WriteCompound(XMLElement* element, Shape* shape, uint32_t handle) {
	int sizex = shape->voxels.sizex;
	int sizey = shape->voxels.sizey;
	int sizez = shape->voxels.sizez;

	Vec3 axis_offset(0.05f * (sizex - sizex % 2), 0.05f * (sizey - sizey % 2), 0);
	axis_offset = axis_offset * (10.0f * shape->voxels.scale);
	Transform shape_transform = shape->transform;
	shape_transform.pos = shape_transform.pos + shape_transform.rot * axis_offset;
	shape_transform.rot = shape_transform.rot * QuatEuler(90, 0, 0);
	shape->transform = shape_transform;

	bool collide = (shape->shape_flags & 0x10) != 0;

	element->SetName("compound");
	xml.AddTransformAttribute(element, shape_transform);
	xml.AddTextureAttribute(element, "texture", shape->texture);
	xml.AddTextureAttribute(element, "blendtexture", shape->blendtexture);
	xml.AddFloatAttribute(element, "density", shape->density, "1");
	xml.AddFloatAttribute(element, "strength", shape->strength, "1");
	xml.AddBoolAttribute(element, "collide", collide, true);

	for (int i = 0; i < (sizex + 256 - 1) / 256; i++)
		for (int j = 0; j < (sizey + 256 - 1) / 256; j++)
			for (int k = 0; k < (sizez + 256 - 1) / 256; k++)
				WriteCompoundShape(element, shape, handle, i, j, k);
}

void WriteXML::WriteCompoundShape(XMLElement* parent, const Shape* shape, uint32_t handle, int i, int j, int k) {
	int offsetx = 256 * i;
	int offsety = 256 * j;
	int offsetz = 256 * k;
	int sizex = shape->voxels.sizex;
	int sizey = shape->voxels.sizey;
	int sizez = shape->voxels.sizez;
	int part_sizex = min(256, sizex - offsetx);
	int part_sizey = min(256, sizey - offsety);
	int part_sizez = min(256, sizez - offsetz);

	float pos_x = 25.6 * i + (part_sizex / 2) * 0.1;
	float pos_y = 25.6 * k;
	float pos_z = -25.6 * j - (part_sizey / 2) * 0.1;
	pos_x -= (sizex / 2) * 0.1;
	pos_z += (sizey / 2) * 0.1;

	int mv_pos_x = 10 * pos_x;
	int mv_pos_y = -10 * pos_z;
	int mv_pos_z = 10 * pos_y + part_sizez / 2 + part_sizez % 2;

	string vox_folder = params.legacy_format ? "custom/" : "vox/";
	string vox_filename ="palette" + to_string(shape->voxels.palette_id) + ".vox";
	string vox_full_path = params.map_folder + vox_folder + vox_filename;
	string path_prefix = params.legacy_format ? "LEVEL/" : "MOD/vox/";
	string vox_path = path_prefix + vox_filename;
	string vox_object = "shape" + to_string(handle) + "_part" + to_string(i) + to_string(j) + to_string(k);

	MV_FILE* vox_file;
	if (vox_files.find(shape->voxels.palette_id) == vox_files.end()) {
		vox_file = new MV_FILE(vox_full_path);
		vox_files[shape->voxels.palette_id] = vox_file;
	} else
		vox_file = vox_files[shape->voxels.palette_id];

	MV_Shape mvshape = { vox_object, mv_pos_x, mv_pos_y, mv_pos_z, Tensor3D(part_sizex, part_sizey, part_sizez) };
	mvshape.voxels.Set(0, 0, 0, 255);
	mvshape.voxels.Set(part_sizex - 1, part_sizey - 1, part_sizez - 1, 255);
	vox_file->SetEntry(255, HOLE_COLOR, HOLE_MATERIAL);

	bool empty = true;
	const Palette& palette = scene.palettes[shape->voxels.palette_id];
	for (int z = offsetz; z < part_sizez + offsetz; z++)
		for (int y = offsety; y < part_sizey + offsety; y++)
			for (int x = offsetx; x < part_sizex + offsetx; x++) {
				uint8_t index = shape->decoded_voxels.Get(x, y, z);
				if (index != 0) {
					// Add voxels that are not snow
					if (!params.remove_snow || index != 254)
						mvshape.voxels.Set(x - offsetx, y - offsety, z - offsetz, index);
					// Add used palette entries
					const Material& palette_entry = palette.materials[index];
					vox_file->SetEntry(index, ToMV(palette_entry.rgba), ToMV(palette_entry));
					empty = false;
				}
			}
	if (!empty) {
		bool duplicated = vox_file->GetShapeName(mvshape, vox_object);
		if (!duplicated)
			vox_file->AddShape(mvshape);

		XMLElement* shape_xml = xml.AddChildElement(parent, "vox");
		xml.AddVec3Attribute(shape_xml, "pos", Vec3(pos_x, pos_y, pos_z), "0 0 0");
		xml.AddStringAttribute(shape_xml, "file", vox_path);
		xml.AddStringAttribute(shape_xml, "object", vox_object);
	}
}

void WriteXML::WriteLight(XMLElement* element, const Light* light, const Entity* parent) {
	Color light_color = light->color;
	light_color.r = pow(light->color.r, 1 / 2.2f);
	light_color.g = pow(light->color.g, 1 / 2.2f);
	light_color.b = pow(light->color.b, 1 / 2.2f);

	element->SetName("light");
	if (parent == nullptr || (parent->type != Entity::Screen && parent->type != Entity::Light))
		xml.AddTransformAttribute(element, GetLocalTransform(parent, light->transform));
	xml.AddStringAttribute(element, "type", LightName[light->type], "sphere");
	xml.AddColorAttribute(element, "color", light_color, "1 1 1 1");
	xml.AddFloatAttribute(element, "scale", light->scale, "1");

	if (light->type == Light::Cone) {
		xml.AddFloatAttribute(element, "angle", 2.0 * deg(acos(light->angle)), "90");
		xml.AddFloatAttribute(element, "penumbra", 2.0 * deg(acos(light->angle) - acos(light->penumbra)), "10");
	}

	if (light->type == Light::Area)
		xml.AddVec2Attribute(element, "size", Vec2(2.0 * light->area_size[0], 2.0 * light->area_size[1]), "0.1 0");
	else if (light->type == Light::Capsule)
		xml.AddVec2Attribute(element, "size", Vec2(2.0 * light->capsule_size, light->size), "0.1 0");
	else
		xml.AddFloatAttribute(element, "size", light->size, "0.1");

	xml.AddFloatAttribute(element, "reach", light->reach, "0");
	xml.AddFloatAttribute(element, "unshadowed", light->unshadowed, "0");
	xml.AddFloatAttribute(element, "fogscale", light->fogscale, "1");
	xml.AddFloatAttribute(element, "fogiter", light->fogiter, "1");
	xml.AddSoundAttribute(element, "sound", light->sound, "");
	xml.AddFloatAttribute(element, "glare", light->glare, "0");
}

void WriteXML::WriteLocation(XMLElement* element, const Location* location, const Entity* parent) {
	element->SetName("location");
	if (parent == nullptr || parent->type != Entity::Trigger)
		xml.AddTransformAttribute(element, GetLocalTransform(parent, location->transform));

	if (element->Parent() == xml.GetScene())
		xml.GetGroupElement(LOCATION)->InsertEndChild(element);
}

void WriteXML::WriteWater(XMLElement* element, const Water* water) {
	element->SetName("water");
	xml.AddTransformAttribute(element, water->transform);
	xml.AddStringAttribute(element, "type", "polygon");
	xml.AddFloatAttribute(element, "depth", water->depth, "10");
	xml.AddFloatAttribute(element, "wave", water->wave, "0.5");
	xml.AddFloatAttribute(element, "ripple", water->ripple, "0.5");
	xml.AddFloatAttribute(element, "motion", water->motion, "0.5");
	xml.AddFloatAttribute(element, "foam", water->foam, "0.5");
	xml.AddColorAttribute(element, "color", water->color, "0.01 0.01 0.01");
	xml.AddFloatAttribute(element, "visibility", water->visibility, "3");
	xml.AddVerticesAttribute(element, water->vertices);

	if (element->Parent() == xml.GetScene())
		xml.GetGroupElement(WATER)->InsertEndChild(element);
}

void WriteXML::WriteJoint(const Joint* joint, string tags) {
	uint32_t shape_handle = joint->shapes[0];
	if (shape_handle == 0) return;
	assert(entity_mapping.find(shape_handle) != entity_mapping.end());
	Entity* parent_entity = entity_mapping[shape_handle];
	XMLElement* parent_element = xml.GetEntityElement(shape_handle);
	assert(parent_element != nullptr);

	Vec3 relative_pos = joint->positions[0];
	Quat relative_rot;
	if (joint->type != Joint::Ball) {
		Vec3 joint_axis(joint->axes[0]);
		if (joint_axis == Vec3(1, 0, 0))
			relative_rot = QuatEuler(0, 90, 0);
		else if (joint_axis == Vec3(-1, 0, 0))
			relative_rot = QuatEuler(0, -90, 0);
		else if (joint_axis == Vec3(0, 1, 0))
			relative_rot = QuatEuler(-90, 0, 0);
		else if (joint_axis == Vec3(0, -1, 0))
			relative_rot = QuatEuler(90, 0, 0);
		else if (joint_axis == Vec3(0, 0, 1))
			relative_rot = Quat();
		else if (joint_axis == Vec3(0, 0, -1))
			relative_rot = QuatEuler(0, 180, 0);
		else {
			double a = -asin(joint_axis.y);
			double b = asin(joint_axis.x / cos(a));
			//double b2 = acos(joint_axis.z / cos(a));
			if (!isnan(a) && !isnan(b))
				relative_rot = QuatEulerRad(a, b, 0);
		}
	}
	Transform joint_tr = Transform(relative_pos, relative_rot);
	joint_tr = GetLocalTransform(parent_entity, joint_tr);

	XMLElement* element = xml.AddChildElement(parent_element, "joint"/*, entity->handle*/);
	xml.AddStringAttribute(element, "tags", tags);
	xml.AddTransformAttribute(element, joint_tr);

	if (joint->type == Joint::Hinge)
		xml.AddStringAttribute(element, "type", "hinge");
	else if (joint->type == Joint::Prismatic)
		xml.AddStringAttribute(element, "type", "prismatic");
	else if (joint->type == Joint::Cone)
		xml.AddStringAttribute(element, "type", "cone");

	xml.AddFloatAttribute(element, "size", joint->size, "0.1");
	if (joint->type != Joint::Prismatic) {
		xml.AddFloatAttribute(element, "rotstrength", joint->rotstrength, "0");
		xml.AddFloatAttribute(element, "rotspring", joint->rotspring, "0.5");
	}
	xml.AddBoolAttribute(element, "collide", joint->collide, false);
	if (joint->type == Joint::Hinge || joint->type == Joint::Cone) {
		Vec2 limits = joint->limits;
		limits.x = deg(limits.x);
		limits.y = deg(limits.y);
		xml.AddVec2Attribute(element, "limits", limits, "0 0");
	} else if (joint->type == Joint::Prismatic)
		xml.AddVec2Attribute(element, "limits", joint->limits, "0 0");
	xml.AddBoolAttribute(element, "sound", joint->sound, false);
	if (joint->type == Joint::Prismatic)
		xml.AddBoolAttribute(element, "autodisable", joint->autodisable, false);
}

void WriteXML::WriteRope(XMLElement* element, const Rope* rope, float size) {
	int knot_count = rope->segments.getSize();
	if (knot_count < 2)
		return;
	Vec3 rope_start = rope->segments[0].from;
	Vec3 rope_end = rope->segments[knot_count - 1].to;
	Vec3 rope_dir = rope_end - rope_start;
	float rope_length = rope_dir.length();
	float slack = rope->slack - rope_length;

	element->SetName("rope");
	xml.AddFloatAttribute(element, "size", size, "0.2");
	xml.AddColorAttribute(element, "color", rope->color, "0 0 0");
	xml.AddFloatAttribute(element, "slack", slack, "0");
	xml.AddFloatAttribute(element, "strength", rope->strength, "1");
	xml.AddFloatAttribute(element, "maxstretch", rope->maxstretch, "0");

	XMLElement* location_from = xml.AddChildElement(element, "location");
	xml.AddVec3Attribute(location_from, "pos", rope_start, "0 0 0");
	XMLElement* location_to = xml.AddChildElement(element, "location");
	xml.AddVec3Attribute(location_to, "pos", rope_end, "0 0 0");

	if (element->Parent() == xml.GetScene())
		xml.GetGroupElement(ROPE)->InsertEndChild(element);
}

void WriteXML::WriteVehicle(XMLElement* element, const Vehicle* vehicle, bool is_boat) {
	element->SetName("vehicle");
	xml.AddTransformAttribute(element, vehicle->transform);
	xml.AddSoundAttribute(element, "sound", vehicle->properties.sound, "medium");
	xml.AddFloatAttribute(element, "spring", vehicle->properties.spring, "1");
	xml.AddFloatAttribute(element, "damping", vehicle->properties.damping, "1");
	xml.AddFloatAttribute(element, "topspeed", 3.6 * vehicle->properties.topspeed, "70");
	xml.AddFloatAttribute(element, "acceleration", vehicle->properties.acceleration, "1");
	xml.AddFloatAttribute(element, "strength", vehicle->properties.strength, "1");
	xml.AddFloatAttribute(element, "antispin", vehicle->properties.antispin, "0");
	xml.AddFloatAttribute(element, "antiroll", vehicle->properties.antiroll, "0");
	xml.AddFloatAttribute(element, "difflock", vehicle->difflock, "0");
	xml.AddFloatAttribute(element, "steerassist", vehicle->properties.steerassist, "0");
	xml.AddFloatAttribute(element, "friction", vehicle->properties.friction, "1.3");

	int exhausts_count = vehicle->exhausts.getSize();
	for (int i = 0; i < exhausts_count; i++) {
		string exhaust_tag = "exhaust";
		if (vehicle->exhausts[i].strength != 0.0f)
			exhaust_tag += "=" + FloatToString(vehicle->exhausts[i].strength);

		XMLElement* exhaust = xml.AddChildElement(element, "location");
		xml.AddStringAttribute(exhaust, "tags", exhaust_tag);
		xml.AddTransformAttribute(exhaust, vehicle->exhausts[i].transform);
	}

	if (!vehicle->player.isZero()) {
		XMLElement* player = xml.AddChildElement(element, "location");
		xml.AddStringAttribute(player, "tags", "player");
		Transform transform;
		transform.pos = vehicle->player;
		transform.rot = QuatEuler(0, 180, 0);
		xml.AddTransformAttribute(player, transform);
	}

	if (!vehicle->camera.isZero() && vehicle->camera != vehicle->player) {
		XMLElement* camera = xml.AddChildElement(element, "location");
		xml.AddStringAttribute(camera, "tags", "camera");
		xml.AddVec3Attribute(camera, "pos", vehicle->camera, "0 0 0");
	}

	if (!vehicle->exit.isZero()) {
		XMLElement* exit = xml.AddChildElement(element, "location");
		xml.AddStringAttribute(exit, "tags", "exit");
		xml.AddVec3Attribute(exit, "pos", vehicle->exit, "0 0 0");
	}

	if (is_boat) {
		XMLElement* propeller = xml.AddChildElement(element, "location");
		xml.AddStringAttribute(propeller, "tags", "propeller");
		xml.AddVec3Attribute(propeller, "pos", vehicle->propeller, "0 0 0");
	}

	if (element->Parent() == xml.GetScene())
		xml.GetGroupElement(VEHICLE)->InsertEndChild(element);
}

void WriteXML::WriteWheel(XMLElement* element, const Wheel* wheel, const Entity* parent) {
	element->SetName("wheel");
	if (parent != nullptr && parent->type == Entity::Body)
		xml.AddTransformAttribute(element, wheel->transform);
	else
		xml.AddTransformAttribute(element, GetLocalTransform(parent, wheel->transform));
	xml.AddFloatAttribute(element, "drive", wheel->drive, "0");
	xml.AddFloatAttribute(element, "steer", wheel->steer, "0");
	xml.AddVec2Attribute(element, "travel", wheel->travel, "-0.1 0.1");
}

void WriteXML::WriteScreen(XMLElement* element, const Screen* screen, const Entity* parent) {
	string script_file = screen->script;
	if (!params.level_id.empty()) {
		string prefix = "data/level/" + params.level_id;
		if (script_file.find(prefix) == 0)
			script_file = "LEVEL" + script_file.substr(prefix.size());
	}
	string prefix = "data/";
	if (script_file.find(prefix) == 0)
		script_file = script_file.substr(prefix.size());

	element->SetName("screen");

	if (parent != nullptr && parent->type == Entity::Shape) {
		Shape* shape = static_cast<Shape*>(parent->self);
		Transform local_transform = TransformToLocalTransform(shape->original_tr, shape->transform);
		local_transform = TransformToLocalTransform(local_transform, screen->transform);
		xml.AddTransformAttribute(element, local_transform);
	}

	xml.AddVec2Attribute(element, "size", screen->size, "0.9 0.5");
	xml.AddFloatAttribute(element, "bulge", screen->bulge, "0.08");
	xml.AddVec2Attribute(element, "resolution", Vec2(screen->resolution[0], screen->resolution[1]), "640 480");
	xml.AddStringAttribute(element, "script", script_file);
	xml.AddBoolAttribute(element, "enabled", screen->enabled, false);
	xml.AddBoolAttribute(element, "interactive", screen->interactive, false);
	xml.AddFloatAttribute(element, "emissive", screen->emissive, "1");
	xml.AddFloatAttribute(element, "fxraster", screen->fxraster, "0");
	xml.AddFloatAttribute(element, "fxca", screen->fxca, "0");
	xml.AddFloatAttribute(element, "fxnoise", screen->fxnoise, "0");
	xml.AddFloatAttribute(element, "fxglitch", screen->fxglitch, "0");
}

void WriteXML::WriteTrigger(XMLElement* element, const Trigger* trigger) {
	element->SetName("trigger");
	Transform trigger_transform = trigger->transform;
	if (trigger->type == Trigger::Box) {
		Vec3 offset = Vec3(0, trigger->box_size.y, 0);
		trigger_transform.pos = trigger_transform.pos - trigger_transform.rot * offset;
	}
	xml.AddTransformAttribute(element, trigger_transform);

	if (trigger->type == Trigger::Sphere)
		xml.AddFloatAttribute(element, "size", trigger->sphere_size, "10");
	else if (trigger->type == Trigger::Box) {
		xml.AddStringAttribute(element, "type", "box");
		xml.AddVec3Attribute(element, "size", trigger->box_size * 2.0, "10 10 10");
	} else if (trigger->type == Trigger::Polygon) {
		xml.AddStringAttribute(element, "type", "polygon");
		xml.AddFloatAttribute(element, "size", trigger->polygon_size, "10");
		xml.AddVerticesAttribute(element, trigger->polygon_vertices);
	}
	xml.AddSoundAttribute(element, "sound", {trigger->sound.path, trigger->sound.volume}, "");
	xml.AddFloatAttribute(element, "soundramp", trigger->sound.ramp, "2");

	if (element->Parent() == xml.GetScene())
		xml.GetGroupElement(TRIGGER)->InsertEndChild(element);
}

void WriteXML::WriteScript(const Script* script) {
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
		return; // Not cooked enough

	if (script_file == "achievements.lua" ||
		script_file == "characters.lua" ||
		script_file == "creativemode.lua" ||
		script_file == "explosion.lua" ||
		script_file == "fx.lua" ||
		script_file == "playerbody.lua" ||
		script_file == "spawn.lua")
		return;

	XMLElement* script_element = xml.AddChildElement(xml.GetGroupElement(SCRIPT), "script"/*, entity->handle*/);
	xml.AddStringAttribute(script_element, "file", script_file);
	for (unsigned int i = 0; i < script->params.getSize(); i++) {
		string param_index = "param" + to_string(i);
		string param = script->params[i].key;
		if (script->params[i].value.length() > 0)
			param += "=" + script->params[i].value;
		xml.AddStringAttribute(script_element, param_index.c_str(), param);
	}

	for (unsigned int i = 0; i < script->entities.getSize(); i++) {
		uint32_t handle = script->entities[i];
		XMLElement* entity_element = xml.GetEntityElement(handle);
		if (entity_element == nullptr)
			continue;

		// Check if some xml parent is also inside the script
		bool script_includes_parent = false;
		XMLElement* parent_element = nullptr;
		if (entity_element->Parent() != nullptr)
			parent_element = entity_element->Parent()->ToElement();
		while (parent_element != nullptr && !script_includes_parent) {
			for (unsigned int j = 0; j < script->entities.getSize(); j++) {
				uint32_t other_handle = script->entities[j];
				if (other_handle == handle)
					continue;
				XMLElement* other_element = xml.GetEntityElement(other_handle);
				if (parent_element == other_element) {
					script_includes_parent = true;
					break;
				}
			}
			if (parent_element->Parent() != nullptr)
				parent_element = parent_element->Parent()->ToElement();
			else
				parent_element = nullptr;
		}
		if (script_includes_parent)
			continue;

		// Move script inside the parent of this entity, if it's not a group
		assert(entity_element->Parent() != nullptr);
		parent_element = entity_element->Parent()->ToElement();
		if (parent_element != xml.GetScene() && parent_element->Name() != string("group"))
			parent_element->InsertEndChild(script_element);
		
		// Move entity inside the script
		script_element->InsertEndChild(entity_element);
	}
}

void WriteXML::WriteAnimator(XMLElement* element, const Animator* animator) {
	element->SetName("animator");
	xml.AddStringAttribute(element, "file", animator->path);
	xml.AddTransformAttribute(element, animator->transform);
}

void WriteXML::WriteEntity(XMLElement* parent, const Entity* entity) {
	XMLElement* element = xml.AddChildElement(parent, EntityName[entity->type], entity->handle);
	xml.AddStringAttribute(element, "tags", ConcatTags(entity->tags));
	xml.AddStringAttribute(element, "desc", entity->desc);

	switch (entity->type) {
		case Entity::Body: {
			Body* body = static_cast<Body*>(entity->self);
			// Add world body as a group
			if (entity->handle == scene.world_body) {
				parent->DeleteChild(element);
				element = xml.GetGroupElement(WORLD_BODY);
				xml.AddTransformAttribute(element, body->transform);
			} else {
				// Skip empty bodies and wheel bodies
				if (entity->children.getSize() == 0 || (entity->parent != nullptr && entity->parent->type == Entity::Wheel)) {
					parent->DeleteChild(element);
					element = parent;
				} else
					WriteBody(element, body, entity->parent);
			}
		}
			break;
		case Entity::Shape: {
			Shape* shape = static_cast<Shape*>(entity->self);
			int volume = shape->voxels.sizex * shape->voxels.sizey * shape->voxels.sizez;
			if (volume == 0) {
				parent->DeleteChild(element);
				element = parent;
			} else
				WriteShape(element, shape, entity->handle);
		}
			break;
		case Entity::Light: {
			Light* light = static_cast<Light*>(entity->self);
			if (entity->parent != nullptr && entity->handle != scene.flashlight)
				WriteLight(element, light, entity->parent);
			else {
				parent->DeleteChild(element);
				element = parent;
			}
		}
			break;
		case Entity::Location: {
			Location* location = static_cast<Location*>(entity->self);
			Entity* entity_parent = entity->parent;
			while (entity_parent != nullptr && entity_parent->type != Entity::Vehicle)
				entity_parent = entity_parent->parent;

			bool generic_location = true;
			bool inside_vehicle = entity_parent != nullptr;
			if (entity->parent != nullptr && entity->parent->type == Entity::Joint) {
				parent->DeleteChild(element);
				element = parent;
				generic_location = false;
			} else if (inside_vehicle) {
				string tags = ConcatTags(entity->tags);
				if (tags == "camera" || tags == "vital" ||
					tags == "exhaust" || tags == "exit" ||
					tags == "propeller") {
					parent->DeleteChild(element);
					element = parent;
					generic_location = false;
				} else if (tags == "player") {
					element->SetName("group");
					element->DeleteAttribute("tags");
					xml.AddStringAttribute(element, "name", "ik");
					xml.AddTransformAttribute(element, GetLocalTransform(entity->parent, location->transform));
					generic_location = false;
				}
			}
			if (generic_location)
				WriteLocation(element, location, entity->parent);
		}
			break;
		case Entity::Water: {
			Water* water = static_cast<Water*>(entity->self);
			WriteWater(element, water);
		}
			break;
		case Entity::Joint: {
			Joint* joint = static_cast<Joint*>(entity->self);
			if (joint->type == Joint::_Rope)
				WriteRope(element, joint->rope, joint->size);
			else {
				parent->DeleteChild(element);
				element = parent;
			}
		}
			break;
		case Entity::Vehicle: {
			bool is_boat = false;
			for (unsigned int i = 0; i < entity->tags.getSize(); i++)
				is_boat |= entity->tags[i].name == "boat";
			Vehicle* vehicle = static_cast<Vehicle*>(entity->self);
			WriteVehicle(element, vehicle, is_boat);
		}
			break;
		case Entity::Wheel: {
			Wheel* wheel = static_cast<Wheel*>(entity->self);
			WriteWheel(element, wheel, entity->parent);
		}
			break;
		case Entity::Script: {
			parent->DeleteChild(element);
			element = parent;
		}
			break;
		case Entity::Screen: {
			Screen* screen = static_cast<Screen*>(entity->self);
			WriteScreen(element, screen, entity->parent);
		}
			break;
		case Entity::Trigger: {
			Trigger* trigger = static_cast<Trigger*>(entity->self);
			WriteTrigger(element, trigger);
		}
			break;
		case Entity::Animator:
			Animator* animator = static_cast<Animator*>(entity->self);
			WriteAnimator(element, animator);
			break;
	}

	for (unsigned int i = 0; i < entity->children.getSize(); i++)
		WriteEntity(element, entity->children[i]);
}

void WriteXML::WriteEntity2ndPass(const Entity* entity) {
	if (entity->type == Entity::Vehicle) {
		Vehicle* vehicle = static_cast<Vehicle*>(entity->self);
		int vital_count = vehicle->vitals.getSize();
		for (int i = 0; i < vital_count; i++) {
			uint32_t body_handle = vehicle->vitals[i].body;
			XMLElement* body_xml = xml.GetEntityElement(body_handle);
			if (body_xml != nullptr) {
				XMLElement* vital = xml.AddChildElement(body_xml, "location");
				xml.AddStringAttribute(vital, "tags", "vital");
				xml.AddVec3Attribute(vital, "pos", vehicle->vitals[i].position, "0 0 0");
			}
		}
	} else if (entity->type == Entity::Joint) {
		Joint* joint = static_cast<Joint*>(entity->self);
		if (joint->type != Joint::_Rope)
			WriteJoint(joint, ConcatTags(entity->tags));
	} else if (entity->type == Entity::Script) {
		Script* script = static_cast<Script*>(entity->self);
		WriteScript(script);
	}

	for (unsigned int i = 0; i < entity->children.getSize(); i++)
		WriteEntity2ndPass(entity->children[i]);
}
