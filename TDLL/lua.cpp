#include "lua.h"

int LuaGetTableField(lua_State* L, const char* name, const char* key) {
	int result = 0;
	lua_getglobal(L, name);
	if (lua_istable(L, -1)) {
		lua_getfield(L, -1, key);
		if (lua_isnumber(L, -1))
			result = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	return result;
}

void LuaSetTableField(lua_State* L, const char* name, const char* key, double value) {
	lua_getglobal(L, name);
	lua_pushnumber(L, value);
	lua_setfield(L, -2, key);
	lua_pop(L, 1);
}

void LuaSetPaletteColor(lua_State* L, int index, double r, double g, double b) {
	lua_getglobal(L, "palette");
	lua_pushnumber(L, index);
	lua_gettable(L, -2);

	lua_pushnumber(L, 1);
	lua_pushnumber(L, r);
	lua_settable(L, -3);

	lua_pushnumber(L, 2);
	lua_pushnumber(L, g);
	lua_settable(L, -3);

	lua_pushnumber(L, 3);
	lua_pushnumber(L, b);
	lua_settable(L, -3);

	lua_pop(L, 2);
}

void LuaSetTableColor(lua_State* L, const char* name, const char* key, int index, double r, double g, double b) {
	lua_getglobal(L, name);
	lua_getfield(L, -1, key);
	lua_pushnumber(L, index);
	lua_gettable(L, -2);

	lua_pushnumber(L, 1);
	lua_pushnumber(L, r);
	lua_settable(L, -3);

	lua_pushnumber(L, 2);
	lua_pushnumber(L, g);
	lua_settable(L, -3);

	lua_pushnumber(L, 3);
	lua_pushnumber(L, b);
	lua_settable(L, -3);

	lua_pop(L, 3);
}
