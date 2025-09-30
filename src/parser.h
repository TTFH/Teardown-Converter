#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdio.h>
#include <atomic>
#include <string>
#include <map>

#include "scene.h"
#include "binary_reader.h"

using namespace std;

extern atomic<float> progress;

struct ConverterParams {
	string bin_path = "";		// Path to the bin/tdbin file
	string map_folder = "";		// Path where the map will be saved
	string script_folder = "";	// Path where the scripts are copied from

	string dlc_id = "";
	string level_id = "";
	string level_name = "";
	string level_desc = "";

	bool use_voxbox = true;
	bool remove_snow = false;
	bool compress_vox = false;
	bool legacy_format = false;

	int transform_precision = 2;
};

void ParseFile(ConverterParams params);

class TDBIN : public FileReader {
protected:
	Scene scene;
	int tdbin_version = 0;
	map<uint32_t, Entity*> entity_mapping;
private:
	Fire ReadFire();
	Palette ReadPalette();
	Rope* ReadRope();
	VehicleProperties ReadVehicleProperties();
	Voxels ReadVoxels();
	LuaValue ReadLuaValue(LuaType key_type);
	LuaTable* ReadLuaTable();

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
	Animator* ReadAnimator();

	void ReadPostProcessing();
	void ReadPlayer();
	void ReadEnvironment();
	void* ReadEntityType(uint8_t type);
public:
	TDBIN();
	void InitScene(const char* input);
	~TDBIN();
	void parse();
};

#endif
