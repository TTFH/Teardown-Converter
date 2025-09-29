#include <stdint.h>
#include <string>

#include "../lib/tinyxml2.h"
#include "entity.h"
#include "misc_utils.h"
#include "xml_writer.h"

static const char* GroupName[] = {"Props", "Locations", "Water", "Ropes", "Vehicles", "Triggers", "Scripts"};

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

XMLElement* XML_Writer::GetScene() {
	return scene;
}

XMLElement* XML_Writer::GetGroupElement(GroupType type) {
	return groups[type];
}

XMLElement* XML_Writer::GetEntityElement(uint32_t handle) {
	if (element_mapping.find(handle) != element_mapping.end())
		return element_mapping[handle];
	return nullptr;
}

XMLElement* XML_Writer::AddChildElement(XMLElement* parent, const char* name, uint32_t handle) {
	XMLElement* element = main_xml.NewElement(name);
	parent->InsertEndChild(element);
	if (handle != 0)
		element_mapping[handle] = element;
	return element;
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
	string buffer = FloatToString(value.x) + " " + FloatToString(value.y) + " " + FloatToString(value.z) + " " +
					FloatToString(value.w);
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
bool XML_Writer::IsChildOf(XMLElement* parent, XMLElement* child) {
	if (parent == nullptr || child == nullptr)
		return false;
	while (child != nullptr) {
		if (child == parent)
			return true;
		if (child->Parent() != nullptr)
			child = child->Parent()->ToElement();
		else
			child = nullptr;
	}
	return false;
}
*/
