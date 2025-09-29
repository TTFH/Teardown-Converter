#include <stdint.h>
#include <string>

#include "entity.h"
#include "xml_writer.h"
#include "misc_utils.h"
#include "../lib/tinyxml2.h"

static const char* GroupName[] = {
	"Props",
	"Locations",
	"Water",
	"Ropes",
	"Vehicles",
	"Triggers",
	"Scripts"
};

XML_Writer::XML_Writer() {
	scene = main_xml.NewElement("scene");
	main_xml.InsertEndChild(scene);

	for (unsigned int i = 0; i < GROUP_COUNT; i++) {
		XMLElement* group = main_xml.NewElement("group");
		group->SetAttribute("name", GroupName[i]);
		main_xml.InsertEndChild(group);
	}
}

void XML_Writer::SaveFile(const char* filename) {
	for (unsigned int i = 0; i < GROUP_COUNT; i++)
		if (groups[i]->NoChildren())
			scene->DeleteChild(groups[i]);
	main_xml.SaveFile(filename);
}

XMLElement* XML_Writer::AddSceneElement(const char* name) {
	XMLElement* element = main_xml.NewElement(name);
	scene->InsertEndChild(element);
	return element;
}

XMLElement* XML_Writer::AddChildElement(XMLElement* parent, const char* name) {
	XMLElement* element = main_xml.NewElement(name);
	parent->InsertEndChild(element);
	return element;
}

void XML_Writer::AddSceneAttributes(uint8_t version[3], Vec3 shadow_volume) {
	string version_str = to_string(version[0]) + "." + to_string(version[1]) + "." + to_string(version[2]);
	scene->SetAttribute("version", version_str.c_str());
	AddVec3Attribute(scene, "shadowVolume", shadow_volume, "100 25 100");
}

void XML_Writer::AddTransformAttribute(XMLElement* element, const Transform& tr) {
	AddVec3Attribute(element, "pos", tr.pos, "0 0 0");
	AddVec3Attribute(element, "rot", QuatToEuler(tr.rot), "0 0 0");
}

void XML_Writer::AddVerticesAttribute(XMLElement* element, const Vec<Vertex>& vertices) {
	for (unsigned int i = 0; i < vertices.getSize(); i++) {
		XMLElement* vertex = main_xml.NewElement("vertex");
		element->InsertEndChild(vertex);
		AddVec2Attribute(vertex, "pos", vertices[i], "0 0");
	}
}

void XML_Writer::AddTextureAttribute(XMLElement* element, const char* name, Texture value) {
	string buffer = to_string(value.tile);
	if (value.weight != 1.0)
		buffer += " " + FloatToString(value.weight);
	if (value.tile != 0)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddBoolAttribute(XMLElement* element, const char* name, bool value, bool default_value) {
	if (value != default_value)
		element->SetAttribute(name, value);
}

void XML_Writer::AddVec2Attribute(XMLElement* element, const char* name, Vec2 value, string default_value) {
	string buffer = FloatToString(value.x) + " " + FloatToString(value.y);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddVec3Attribute(XMLElement* element, const char* name, Vec3 value, string default_value) {
	string buffer = FloatToString(value.x) + " " + FloatToString(value.y) + " " + FloatToString(value.z);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddVec4Attribute(XMLElement* element, const char* name, Vec4 value, string default_value) {
	string buffer = FloatToString(value.x) + " " + FloatToString(value.y) + " " + FloatToString(value.z) + " " + FloatToString(value.w);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddColorAttribute(XMLElement* element, const char* name, Color value, string default_value) {
	string buffer = FloatToString(value.r) + " " + FloatToString(value.g) + " " + FloatToString(value.b);
	if (value.a != 0.0 && value.a != 1.0)
		buffer += " " + FloatToString(value.a);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddSoundAttribute(XMLElement* element, const char* name, Sound value, string default_value) {
	string buffer = value.path;
	if (value.volume != 1.0)
		buffer += " " + FloatToString(value.volume);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloatAttribute(XMLElement* element, const char* name, float value, string default_value) {
	string buffer = FloatToString(value);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddStringAttribute(XMLElement* element, const char* name, string value, string default_value) {
	if (value != default_value)
		element->SetAttribute(name, value.c_str());
}








/*
XMLElement* XML_Writer::GetScene() {
	return scene;
}

XMLElement* XML_Writer::GetGroup(GroupType type) {
	return groups[type];
}

XMLElement* XML_Writer::GetNode(uint32_t handle) {
	if (element_mapping.find(handle) != element_mapping.end())
		return element_mapping[handle];
	return NULL;
}

XMLElement* XML_Writer::CreateElement(const char* element_name) {
	return main_xml->NewElement(element_name);
}

void XML_Writer::AddElement(XMLElement* parent, XMLElement* child, uint32_t handle) {
	parent->InsertEndChild(child);
	if (handle != 0)
		element_mapping[handle] = child;
}

void XML_Writer::MoveElement(XMLElement* oldparent, XMLElement* child) {
	oldparent->InsertEndChild(child);
}

bool XML_Writer::IsChildOf(XMLElement* parent, XMLElement* child) {
	if (parent == NULL || child == NULL)
		return false;
	while (child != NULL) {
		if (child == parent)
			return true;
		if (child->Parent() != NULL)
			child = child->Parent()->ToElement();
		else
			child = NULL;
	}
	return false;
}

void XML_Writer::AddAttribute(XMLElement* element, const char* name, const char* value) {
	element->SetAttribute(name, value);
}

void XML_Writer::AddBoolAttribute(XMLElement* element, const char* name, bool value, bool default_value) {
	if (value != default_value)
		element->SetAttribute(name, value);
}

void XML_Writer::AddFloatAttribute(XMLElement* element, const char* name, float value, string default_value) {
	if (FloatToString(value) != default_value)
		element->SetAttribute(name, FloatToString(value).c_str());
}

// Used for textures
void XML_Writer::AddIntFloatAttribute(XMLElement* element, const char* name, int value1, float value2) {
	if (value2 != 1.0) {
		string buffer = to_string(value1) + " " + FloatToString(value2);
		element->SetAttribute(name, buffer.c_str());
	} else if (value1 != 0)
		element->SetAttribute(name, value1);
}

void XML_Writer::AddStrAttribute(XMLElement* element, const char* name, string value, string default_value) {
	if (value != default_value)
		element->SetAttribute(name, value.c_str());
}

// Used for sounds
void XML_Writer::AddStrFloatAttribute(XMLElement* element, const char* name, string value1, float value2, string default_value) {
	if (value2 != 1.0) {
		string buffer = value1 + " " + FloatToString(value2);
		element->SetAttribute(name, buffer.c_str());
	} else if (value1 != default_value)
		element->SetAttribute(name, value1.c_str());
}

void XML_Writer::AddVectorAttribute(XMLElement* element, const char* name, Vec3 value, string default_value) {
	string buffer = FloatToString(value.x) + " " + FloatToString(value.y) + " " + FloatToString(value.z);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat2Attribute(XMLElement* element, const char* name, float value1, float value2, string default_value) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat3Attribute(XMLElement* element, const char* name, float value1, float value2, float value3) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2) + " " + FloatToString(value3);
	element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat4Attribute(XMLElement* element, const char* name, float value1, float value2, float value3, float value4, string default_value) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2) + " " + FloatToString(value3) + " " + FloatToString(value4);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloatNAttribute(XMLElement* element, const char* name, const float* value, int count, string default_value) {
	string buffer = FloatToString(value[0]);
	if (count > 1)
		buffer += " " + FloatToString(value[1]);
	if (count > 2)
		buffer += " " + FloatToString(value[2]);
	if (count > 3)
		buffer += " " + FloatToString(value[3]);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddColorAttribute(XMLElement* element, const char* name, Color value, string default_value) {
	string buffer = FloatToString(value.r) + " " + FloatToString(value.g) + " " + FloatToString(value.b);
	if (value.a != 0.0 && value.a != 1.0)
		buffer += " " + FloatToString(value.a);
	if (buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}
*/