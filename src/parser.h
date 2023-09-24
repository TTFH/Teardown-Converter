#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <map>

#include "scene.h"

using namespace std;

extern volatile float progress;

struct ConverterParams {
	string bin_path;
	string map_folder;
	string game_folder;

	string level_id;
	string level_name;
	string level_desc;

	bool remove_snow;
	bool compress_vox;
	bool legacy_format;
};

void ParseFile(ConverterParams params);

class TDBIN {
protected:
	Scene scene;
	int tdbin_version = 0;
	map<uint32_t, Entity*> entity_mapping;
private:
	FILE* bin_file;

	uint8_t ReadByte();
	uint16_t ReadWord();
	uint32_t ReadInt();
	float ReadFloat();
	double ReadDouble();
	string ReadString();

	uint16_t ReadEntityFlags();
	Registry ReadRegistry();
	Color ReadColor();
	Vector ReadVector();
	Transform ReadTransform();

	Fire ReadFire();
	Palette ReadPalette();
	Rope ReadRope();
	Tag ReadTag();
	VehicleProperties ReadVehicleProperties();
	Voxels ReadVoxels();

	LuaValue ReadLuaValue(uint32_t);
	LuaTable ReadLuaTable();

	Entity* ReadEntity();
	Body* ReadBody();
	Shape* ReadShape();
	Light* ReadLight();
	Location* ReadLocation();
	Water* ReadWater();
	Joint* ReadJoint();
	Vehicle* ReadVehicle();
	Wheel* ReadWheel();
	Screen* ReadScreen();
	Trigger* ReadTrigger();
	Script* ReadScript();

	void ReadPlayer();
	void ReadEnvironment();
	void* ReadEntityKind(uint8_t);
public:
	void init(const char* input);
	~TDBIN();
	void parse();
};

#endif
