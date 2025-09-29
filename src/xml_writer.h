#ifndef XML_WRITER_H
#define XML_WRITER_H

#include <stdint.h>
#include <map>
#include <string>
#include <array>
#include "../lib/tinyxml2.h"

using namespace std;
using namespace tinyxml2;

enum GroupType {
	PROP = 0,
	LOCATION,
	WATER,
	ROPE,
	VEHICLE,
	TRIGGER,
	SCRIPT
};

class XML_Writer {
private:
	XMLDocument main_xml;
	XMLElement* scene;
	static const unsigned int GROUP_COUNT = 7;
	array<XMLElement*, GROUP_COUNT> groups;
	map<uint32_t, XMLElement*> element_mapping;
public:
	XML_Writer();
	void SaveFile(const char* filename);
	XMLElement* AddSceneElement(const char* name);
	XMLElement* AddChildElement(XMLElement* parent, const char* name);

	void AddSceneAttributes(uint8_t version[3], Vec3 shadow_volume);
	void AddTransformAttribute(XMLElement* element, const Transform& tr);
	void AddVerticesAttribute(XMLElement* element, const Vec<Vertex>& vertices);
	void AddTextureAttribute(XMLElement* element, const char* name, Texture value);
	void AddBoolAttribute(XMLElement* element, const char* name, bool value, bool default_value);
	void AddVec2Attribute(XMLElement* element, const char* name, Vec2 value, string default_value);
	void AddVec3Attribute(XMLElement* element, const char* name, Vec3 value, string default_value);
	void AddVec4Attribute(XMLElement* element, const char* name, Vec4 value, string default_value);
	void AddColorAttribute(XMLElement* element, const char* name, Color value, string default_value);
	void AddSoundAttribute(XMLElement* element, const char* name, Sound value, string default_value);
	void AddFloatAttribute(XMLElement* element, const char* name, float value, string default_value);
	void AddStringAttribute(XMLElement* element, const char* name, string value, string default_value = "");
	


	//GetScene
	//GetEntityElement
	
	/*void AddElement(XMLElement* parent, XMLElement* child, uint32_t handle = 0);
	void MoveElement(XMLElement* new_parent, XMLElement* child);
	bool IsChildOf(XMLElement* parent, XMLElement* child);
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
	void AddColorAttribute(XMLElement* element, const char* name, Color value, string default_value = "");*/
};

#endif
