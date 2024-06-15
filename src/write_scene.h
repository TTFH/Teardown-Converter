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

	void WriteEntity2ndPass(Entity*);
	void WriteEntity(XMLElement*, Entity*);
	void WriteTransform(XMLElement*, Transform);
	void WriteShape(XMLElement*&, XMLElement*&, Entity*);
	void WriteVertices(XMLElement* parent, const Vec<Vertex> &vertices);
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
