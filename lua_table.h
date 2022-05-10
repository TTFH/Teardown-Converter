#ifndef LUATABLE_H
#define LUATABLE_H

#include <stdint.h>
#include <string.h>
#include <vector>

using namespace std;

enum LuaType { // uint32_t
	NIL = 0,
	Boolean = 1,
	Number = 3,
	String = 4,
	Table = 5,
	Reference = 0xFFFFFFFB
};

struct LuaTableEntry;
typedef vector<LuaTableEntry> LuaTable;

union LuaValue {
	bool Boolean;
	double Number;
	LuaTable* Table;
	char* String;
	uint32_t Reference;
};

struct LuaTableEntry {
	uint32_t key_type;
	LuaValue key; // Only if key_type != NIL
	uint32_t value_type; // Only if key_type != NIL
	LuaValue value; // Only if key_type != NIL

	LuaTableEntry();
	LuaTableEntry(const LuaTableEntry& other);
	LuaTableEntry& operator=(const LuaTableEntry& other) = delete;
	~LuaTableEntry();
};

#endif
