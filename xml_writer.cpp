#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <map>
#include <string>

#include "entity.h"
#include "xml_writer.h"
#include "lib/tinyxml2.h"

using namespace std;
using namespace tinyxml2;

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
	char buffer[32];
	int ret = snprintf(buffer, sizeof(buffer), "%.3g", value + 0);
	assert(ret >= 0 && ret < 32);
	element->SetAttribute(name, buffer);
}

void XML_Writer::AddIntFloatAttribute(XMLElement* element, const char* name, int value1, float value2) {
	char buffer[32];
	int ret = snprintf(buffer, sizeof(buffer), "%d %g", value1, value2);
	assert(ret >= 0 && ret < 32);
	element->SetAttribute(name, buffer);
}

void XML_Writer::AddStrAttribute(XMLElement* element, const char* name, string value) {
	element->SetAttribute(name, value.c_str());
}

void XML_Writer::AddStrFloatAttribute(XMLElement* element, const char* name, string value1, float value2) {
	char buffer[64];
	int ret = snprintf(buffer, sizeof(buffer), "%s %.3g", value1.c_str(), value2 + 0);
	assert(ret >= 0 && ret < 64);
	element->SetAttribute(name, buffer);
}

void XML_Writer::AddFloat2Attribute(XMLElement* element, const char* name, float value1, float value2) {
	char buffer[32];
	int ret = snprintf(buffer, sizeof(buffer), "%g %g", value1, value2);
	assert(ret >= 0 && ret < 32);
	element->SetAttribute(name, buffer);
}

void XML_Writer::AddFloat3Attribute(XMLElement* element, const char* name, float value1, float value2, float value3) {
	char buffer[32];
	int ret = snprintf(buffer, sizeof(buffer), "%g %g %g", value1, value2, value3);
	assert(ret >= 0 && ret < 32);
	element->SetAttribute(name, buffer);
}

void XML_Writer::AddFloat4Attribute(XMLElement* element, const char* name, float value1, float value2, float value3, float value4) {
	char buffer[32];
	int ret = snprintf(buffer, sizeof(buffer), "%g %g %g %g", value1, value2, value3, value4);
	assert(ret >= 0 && ret < 32);
	element->SetAttribute(name, buffer);
}

void XML_Writer::AddFloatNAttribute(XMLElement* element, const char* name, const float* value, int count) {
	char buffer[32];
	int ret = 0;
	if (count == 2)
		ret = snprintf(buffer, sizeof(buffer), "%.3g %.3g", value[0] + 0, value[1] + 0);
	else if (count == 3)
		ret = snprintf(buffer, sizeof(buffer), "%.3g %.3g %.3g", value[0] + 0, value[1] + 0, value[2] + 0);
	else if (count == 4)
		ret = snprintf(buffer, sizeof(buffer), "%.3g %.3g %.3g %.3g", value[0] + 0, value[1] + 0, value[2] + 0, value[3] + 0);
	else
		assert(false);
	assert(ret >= 0 && ret < 32);
	element->SetAttribute(name, buffer);
}

void XML_Writer::AddRgbaAttribute(XMLElement* element, const char* name, Rgba value, bool skip_alpha) {
	char buffer[32];
	int ret = 0;
	if (skip_alpha)
		ret = snprintf(buffer, sizeof(buffer), "%g %g %g", value.r, value.g, value.b);
	else
		ret = snprintf(buffer, sizeof(buffer), "%g %g %g %g", value.r, value.g, value.b, value.a);
	assert(ret >= 0 && ret < 32);
	element->SetAttribute(name, buffer);
}
