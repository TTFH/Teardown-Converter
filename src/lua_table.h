#ifndef LUATABLE_H
#define LUATABLE_H

#include <stdint.h>
#include <vector>

using namespace std;

enum LuaType { // uint32_t
	NIL = 0,
	Boolean = 1,
	Number = 3,
	String = 4,
	Table = 5,
	Reference = 0xFFFFFFFB // -5
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
	LuaValue key;
	uint32_t value_type;
	LuaValue value;

	LuaTableEntry();
	LuaTableEntry(const LuaTableEntry& other);
	LuaTableEntry& operator=(const LuaTableEntry& other) = delete;
	~LuaTableEntry();
};

#endif
