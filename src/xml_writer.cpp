#include <stdint.h>
#include <string>

#include "entity.h"
#include "xml_writer.h"
#include "misc_utils.h"
#include "../lib/tinyxml2.h"

XML_Writer::XML_Writer() {
	main_xml = new XMLDocument();
	scene = main_xml->NewElement("scene");
	main_xml->InsertFirstChild(scene);
}

void XML_Writer::PostInit() {
	props = main_xml->NewElement("group");
	AddStrAttribute(props, "name", "Props");
	AddElement(scene, props);

	locations = main_xml->NewElement("group");
	AddStrAttribute(locations, "name", "Locations");
	AddElement(scene, locations);

	water = main_xml->NewElement("group");
	AddStrAttribute(water, "name", "Water");
	AddElement(scene, water);

	ropes = main_xml->NewElement("group");
	AddStrAttribute(ropes, "name", "Ropes");
	AddElement(scene, ropes);

	vehicles = main_xml->NewElement("group");
	AddStrAttribute(vehicles, "name", "Vehicles");
	AddElement(scene, vehicles);

	triggers = main_xml->NewElement("group");
	AddStrAttribute(triggers, "name", "Triggers");
	AddElement(scene, triggers);

	scripts = main_xml->NewElement("group");
	AddStrAttribute(scripts, "name", "Scripts");
	AddElement(scene, scripts);
}

void XML_Writer::ClearEmptyGroups() {
	if (props->NoChildren())
		scene->DeleteChild(props);
	if (locations->NoChildren())
		scene->DeleteChild(locations);
	if (water->NoChildren())
		scene->DeleteChild(water);
	if (ropes->NoChildren())
		scene->DeleteChild(ropes);
	if (vehicles->NoChildren())
		scene->DeleteChild(vehicles);
	if (triggers->NoChildren())
		scene->DeleteChild(triggers);
	if (scripts->NoChildren())
		scene->DeleteChild(scripts);
}

XML_Writer::~XML_Writer() {
	delete main_xml;
}

void XML_Writer::SaveFile(const char* filename) {
	main_xml->SaveFile(filename);
}

XMLElement* XML_Writer::GetScene() {
	return scene;
}

XMLElement* XML_Writer::GetDynamicGroup() {
	return props;
}

XMLElement* XML_Writer::GetLocationsGroup() {
	return locations;
}

XMLElement* XML_Writer::GetWaterGroup() {
	return water;
}

XMLElement* XML_Writer::GetRopesGroup() {
	return ropes;
}

XMLElement* XML_Writer::GetVehiclesGroup() {
	return vehicles;
}

XMLElement* XML_Writer::GetTriggersGroup() {
	return triggers;
}

XMLElement* XML_Writer::GetScriptsGroup() {
	return scripts;
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
	if (default_value == "" || FloatToString(value) != default_value)
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
	if (default_value == "" || buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat2Attribute(XMLElement* element, const char* name, float value1, float value2, string default_value) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2);
	if (default_value == "" || buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat3Attribute(XMLElement* element, const char* name, float value1, float value2, float value3) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2) + " " + FloatToString(value3);
	element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat4Attribute(XMLElement* element, const char* name, float value1, float value2, float value3, float value4, string default_value) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2) + " " + FloatToString(value3) + " " + FloatToString(value4);
	if (default_value == "" || buffer != default_value)
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
	if (default_value == "" || buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddColorAttribute(XMLElement* element, const char* name, Color value, string default_value) {
	string buffer = FloatToString(value.r) + " " + FloatToString(value.g) + " " + FloatToString(value.b);
	if (value.a != 0.0 && value.a != 1.0)
		buffer += " " + FloatToString(value.a);
	if (default_value == "" || buffer != default_value)
		element->SetAttribute(name, buffer.c_str());
}
