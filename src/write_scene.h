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

	void WriteEntity2ndPass(const Entity* entity);
	void WriteEntity(XMLElement* parent, const Entity* entity);

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
