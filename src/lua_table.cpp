#include "lua_table.h"

LuaTableEntry::LuaTableEntry() {
	key_type = NIL;
	value_type = NIL;
}

LuaTableEntry::~LuaTableEntry() {
	if (key_type == String)
		delete[] key.String;
	else if (key_type == Table) {
		for (LuaTable::iterator it = key.Table->begin(); it != key.Table->end(); it++)
			delete *it;
		delete key.Table;
	}
	if (value_type == String)
		delete[] value.String;
	else if (value_type == Table) {
		for (LuaTable::iterator it = value.Table->begin(); it != value.Table->end(); it++)
			delete *it;
		delete value.Table;
	}
}
