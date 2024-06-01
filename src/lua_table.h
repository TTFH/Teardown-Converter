#ifndef LUATABLE_H
#define LUATABLE_H

#include <stdint.h>
#include <vector>

using namespace std;

enum LuaType : uint32_t {
	NIL = 0,
	Boolean = 1,
	//LightUserData = 2,
	Number = 3,
	String = 4,
	Table = 5,
	//Function = 6,
	//UserData = 7,
	//Thread = 8,
	Reference = 0xFFFFFFFB // -5
};

struct LuaTableEntry;
typedef vector<LuaTableEntry*> LuaTable;

union LuaValue {
	bool Boolean;
	double Number;
	char* String;
	LuaTable* Table;
	uint32_t Reference;
};

struct LuaTableEntry {
	LuaType key_type;
	LuaValue key;
	LuaType value_type;
	LuaValue value;

	LuaTableEntry();
	~LuaTableEntry();
};

#endif
