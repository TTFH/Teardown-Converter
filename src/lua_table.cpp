#include <string.h>

#include "lua_table.h"

LuaTableEntry::LuaTableEntry() {
	key_type = NIL;
	value_type = NIL;
}

LuaTableEntry::LuaTableEntry(const LuaTableEntry& other) {
	key_type = other.key_type;
	if (key_type == String) {
		key.String = new char[strlen(other.key.String) + 1];
		strcpy(key.String, other.key.String);
	} else if (key_type == Table) {
		key.Table = new LuaTable;
		for (unsigned int i = 0; i < other.key.Table->size(); i++)
			key.Table->push_back((*other.key.Table)[i]);
	}

	value_type = other.value_type;
	if (value_type == String) {
		value.String = new char[strlen(other.value.String) + 1];
		strcpy(value.String, other.value.String);
	} else if (value_type == Table) {
		value.Table = new LuaTable;
		for (unsigned int i = 0; i < other.value.Table->size(); i++)
			value.Table->push_back((*other.value.Table)[i]);
	}
}

LuaTableEntry::~LuaTableEntry() {
	if (key_type == String)
		delete[] key.String;
	else if (key_type == Table)
		delete key.Table;
	if (value_type == String)
		delete[] value.String;
	else if (value_type == Table)
		delete value.Table;
}
