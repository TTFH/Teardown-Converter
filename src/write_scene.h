#ifndef WRITE_SCENE_H
#define WRITE_SCENE_H

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "parser.h"
#include "xml_writer.h"

class MV_FILE;
namespace tinyxml2 { class XMLElement; }

using namespace std;

class WriteXML : public TDBIN {
private:
	XML_Writer xml;
	ConverterParams params;
	map<uint32_t, MV_FILE*> vox_files;

	void WriteBody(XMLElement* element, const Body* body, const Entity* parent);
	void WriteShape(XMLElement* element, const Shape* shape, uint32_t handle);
	void WriteLight(XMLElement* element, const Light* light, const Entity* parent);
	void WriteLocation(XMLElement* element, const Location* location, Entity* parent);
	void WriteWater(XMLElement* element, const Water* water);
	void WriteJoint(const Joint* joint, string tags);
	void WriteVehicle(XMLElement* element, const Vehicle* vehicle, bool is_boat);
	void WriteWheel(XMLElement* element, const Wheel* wheel, const Entity* parent);
	void WriteScreen(XMLElement* element, const Screen* screen);
	void WriteTrigger(XMLElement* element, const Trigger* trigger);
	void WriteScript(const Script* script);
	void WriteAnimator(XMLElement* element, const Animator* animator);

	void WriteEntity2ndPass(const Entity* entity);
	void WriteEntity(XMLElement* parent, const Entity* entity);

	void WriteRope(XMLElement* element, const Rope* rope, float size);
	void WriteVox(XMLElement* element, const Shape* shape, uint32_t handle);
	void WriteVoxbox(XMLElement* element, const Shape* shape);
	void WriteCompound(XMLElement* element, const Shape* shape, uint32_t handle);
	void WriteCompoundShape(XMLElement* parent, const Shape* shape, uint32_t handle, int i, int j, int k);
public:
	WriteXML(ConverterParams params);
	~WriteXML();

	void WriteScene();
	void WriteSpawnpoint();
	void WriteEnvironment();
	void WriteBoundary();
	void WritePostProcessing();

	void WriteEntities();
	void SaveXML();
	void SaveVoxFiles();
};

#endif
