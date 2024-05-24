#include <string.h>

#include "lua_table.h"

LuaTableEntry::LuaTableEntry() {
	key_type = NIL;
	value_type = NIL;
}

LuaTableEntry::~LuaTableEntry() {
	if (key_type == String)
		delete[] key.String;
	else if (key_type == Table) // TODO: iterate?
		delete key.Table;
	if (value_type == String)
		delete[] value.String;
	else if (value_type == Table)
		delete value.Table;
}
