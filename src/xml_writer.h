#ifndef XML_WRITER_H
#define XML_WRITER_H

#include <stdint.h>
#include <map>
#include <string>
#include "../lib/tinyxml2.h"

using namespace std;
using namespace tinyxml2;

struct Color;

string FloatToString(float value);

class XML_Writer {
private:
	XMLDocument* main_xml;
	XMLElement* scene;
	XMLElement* props;
	XMLElement* locations;
	XMLElement* water;
	XMLElement* ropes;
	XMLElement* vehicles;
	XMLElement* triggers;
	XMLElement* scripts;
	map<uint32_t, XMLElement*> element_mapping;
public:
	XML_Writer();
	void PostInit();
	void ClearEmptyGroups();
	~XML_Writer();
	void SaveFile(const char* filename);
	XMLElement* GetScene();
	XMLElement* GetDynamicGroup();
	XMLElement* GetLocationsGroup();
	XMLElement* GetWaterGroup();
	XMLElement* GetRopesGroup();
	XMLElement* GetVehiclesGroup();
	XMLElement* GetTriggersGroup();
	XMLElement* GetScriptsGroup();
	XMLElement* GetNode(uint32_t handle);
	XMLElement* CreateElement(const char* element_name);

	void AddElement(XMLElement* parent, XMLElement* child, uint32_t handle = 0);
	void MoveElement(XMLElement* new_parent, XMLElement* child);
	void AddBoolAttribute(XMLElement* element, const char* name, bool value, bool default_value);
	void AddAttribute(XMLElement* element, const char* name, const char* value);
	void AddStrAttribute(XMLElement* element, const char* name, string value, string default_value = "");
	void AddIntFloatAttribute(XMLElement* element, const char* name, int value1, float value2);
	void AddStrFloatAttribute(XMLElement* element, const char* name, string value1, float value2, string default_value = "");
	void AddFloatAttribute(XMLElement* element, const char* name, float value, string default_value = "");
	void AddVectorAttribute(XMLElement* element, const char* name, Vec3 value, string default_value = "");
	void AddFloat2Attribute(XMLElement* element, const char* name, float value1, float value2, string default_value = "");
	void AddFloat3Attribute(XMLElement* element, const char* name, float value1, float value2, float value3);
	void AddFloat4Attribute(XMLElement* element, const char* name, float value1, float value2, float value3, float value4, string default_value = "");
	void AddFloatNAttribute(XMLElement* element, const char* name, const float* value, int count, string default_value = "");
	void AddColorAttribute(XMLElement* element, const char* name, Color value, string default_value = "");
};

#endif
