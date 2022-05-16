#include <math.h>
#include <stdio.h>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "entity.h"
#include "xml_writer.h"
#include "lib/tinyxml2.h"

using namespace std;
using namespace tinyxml2;

static string FloatToString(float value) {
	if (fabs(value) < 0.001) value = 0;
	stringstream ss;
	ss << fixed << setprecision(3) << value;
	string str = ss.str();
	if (str.find('.') != string::npos) {
		str = str.substr(0, str.find_last_not_of('0') + 1);
		if (str.find('.') == str.size() - 1)
			str = str.substr(0, str.size() - 1);
	}
	return str;
}

XML_Writer::XML_Writer() {
	main_xml = new XMLDocument();
	scene = main_xml->NewElement("scene");
	main_xml->InsertFirstChild(scene);
}

XML_Writer::~XML_Writer() {
	delete main_xml;
}

void XML_Writer::SaveFile(const char* filename) {
	main_xml->SaveFile(filename);
}

XMLElement* XML_Writer::getScene() {
	return scene;
}

XMLElement* XML_Writer::getNode(uint32_t handle) {
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

void XML_Writer::AddAttribute(XMLElement* element, const char* name, const char* value) {
	element->SetAttribute(name, value);
}

void XML_Writer::AddBoolAttribute(XMLElement* element, const char* name, bool value) {
	element->SetAttribute(name, value);
}

void XML_Writer::AddFloatAttribute(XMLElement* element, const char* name, float value) {
	element->SetAttribute(name, FloatToString(value).c_str());
}

// Used for textures
void XML_Writer::AddIntFloatAttribute(XMLElement* element, const char* name, int value1, float value2) {
	string buffer = to_string(value1) + " " + FloatToString(value2);
	element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddStrAttribute(XMLElement* element, const char* name, string value) {
	element->SetAttribute(name, value.c_str());
}

// Used for sounds
void XML_Writer::AddStrFloatAttribute(XMLElement* element, const char* name, string value1, float value2) {
	string buffer = value1 + " " + FloatToString(value2);
	element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat2Attribute(XMLElement* element, const char* name, float value1, float value2) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2);
	element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat3Attribute(XMLElement* element, const char* name, float value1, float value2, float value3) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2) + " " + FloatToString(value3);
	element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloat4Attribute(XMLElement* element, const char* name, float value1, float value2, float value3, float value4) {
	string buffer = FloatToString(value1) + " " + FloatToString(value2) + " " + FloatToString(value3) + " " + FloatToString(value4);
	element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddFloatNAttribute(XMLElement* element, const char* name, const float* value, int count) {
	string buffer = FloatToString(value[0]);
	if (count > 1)
		buffer += " " + FloatToString(value[1]);
	if (count > 2)
		buffer += " " + FloatToString(value[2]);
	if (count > 3)
		buffer += " " + FloatToString(value[3]);
	element->SetAttribute(name, buffer.c_str());
}

void XML_Writer::AddRgbaAttribute(XMLElement* element, const char* name, Rgba value, bool skip_alpha) {
	string buffer = FloatToString(value.r) + " " + FloatToString(value.g) + " " + FloatToString(value.b);
	if (!skip_alpha && value.a != 1.0)
		buffer += " " + FloatToString(value.a);
	element->SetAttribute(name, buffer.c_str());
}
