#ifndef WRITE_SCENE_H
#define WRITE_SCENE_H

#include <stdint.h>
#include <list>
#include <map>
#include <string>

#include "entity.h"
#include "scene.h"
#include "xml_writer.h"

class MV_FILE;
namespace tinyxml2 { class XMLElement; }

using namespace std;

class WriteXML{
protected:
	Scene scene;
	XML_Writer xml;
	string save_path;
	string level_id;
	list<MV_FILE*> compound_files;
	map<uint32_t, MV_FILE*> vox_files;
	map<uint32_t, Entity*> entity_mapping;

	void WriteEntity2ndPass(Entity*);
	void WriteEntity(XMLElement*, Entity*);
	void WriteTransform(XMLElement*, Transform);
	void WriteShape(XMLElement*&, Shape*, uint32_t);
	void WriteCompound(MV_FILE*, string, XMLElement*, Shape*, int, int, int);
public:
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
