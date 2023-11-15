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
	vector<MV_FILE*> compound_files;
	map<uint32_t, MV_FILE*> vox_files;

	void WriteEntity2ndPass(Entity*);
	void WriteEntity(XMLElement*, Entity*);
	void WriteTransform(XMLElement*, Transform);
	void WriteShape(XMLElement*&, Shape*, uint32_t);
	void WriteCompound(uint32_t, const Tensor3D&, MV_FILE*, string, XMLElement*, Shape*, int, int, int);
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
