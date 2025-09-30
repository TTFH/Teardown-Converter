#ifndef XML_WRITER_H
#define XML_WRITER_H

#include "../lib/tinyxml2.h"
#include <array>
#include <map>
#include <stdint.h>
#include <string>

using namespace std;
using namespace tinyxml2;

enum GroupType { WORLD_BODY, STATIC, PROP, LOCATION, WATER, ROPE, VEHICLE, TRIGGER, SCRIPT };

class XML_Writer {
private:
	XMLDocument main_xml;
	XMLElement* scene;
	static const unsigned int GROUP_COUNT = 9;
	array<XMLElement*, GROUP_COUNT> groups;
	map<uint32_t, XMLElement*> element_mapping;
	const unsigned int DEFAULT_PRECISION = 2;
	int precision = DEFAULT_PRECISION;
	int transform_precision = DEFAULT_PRECISION;
	string FloatToString(float value);
public:
	XML_Writer();
	void CreateGroups();
	void SaveFile(const char* filename);
	void SetTransformPrecision(int precision);

	XMLElement* GetScene();
	XMLElement* GetGroupElement(GroupType type);
	XMLElement* GetEntityElement(uint32_t handle);

	XMLElement* AddChildElement(XMLElement* parent, const char* name);
	XMLElement* CreateDetachedElement(const char* name);
	void AddEntityElement(XMLElement* parent, XMLElement* child, uint32_t handle);

	void AddExhaustTagAttribute(XMLElement* element, float strength);
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
};

#endif
