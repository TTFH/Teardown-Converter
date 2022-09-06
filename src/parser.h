#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdio.h>
#include <iosfwd>
#include <string>

#include "entity.h"
#include "lua_table.h"
#include "math_utils.h"
#include "scene.h"
#include "write_scene.h"

using namespace std;

extern float progress;

void ParseFile(ConverterParams params);

// TODO: inheritance should be the other way around
class TDBIN : public WriteXML {
private:
	FILE* bin_file;

	uint8_t ReadByte();
	uint16_t ReadWord();
	uint32_t ReadInt();
	float ReadFloat();
	double ReadDouble();
	string ReadString();

	Registry ReadRegistry();
	Rgba ReadRgba();
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
	TDBIN(const char* filename);
	TDBIN(const ConverterParams& params);
	~TDBIN();
	void parse();
};

#endif
