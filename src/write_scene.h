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

struct ConverterParams {
	string bin_path;
	string map_folder;
	string game_folder;

	string level_id;
	string level_name;
	string level_desc;

	bool remove_snow;
	bool xml_only;
	bool compress_vox;
};

class WriteXML {
protected:
	Scene scene;
	XML_Writer xml;
	ConverterParams params;
	list<MV_FILE*> compound_files;
	map<uint32_t, MV_FILE*> vox_files;
	map<uint32_t, Entity*> entity_mapping;

	void WriteEntity2ndPass(Entity*);
	void WriteEntity(XMLElement*, Entity*);
	void WriteTransform(XMLElement*, Transform);
	void WriteShape(XMLElement*&, Shape*, uint32_t, string);
	void WriteCompound(uint32_t, const Tensor3D&, MV_FILE*, string, XMLElement*, Shape*, int, int, int);
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
