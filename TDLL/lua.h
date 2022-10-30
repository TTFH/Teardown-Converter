extern "C" {
    #include <lua5.1/lualib.h>
    #include <lua5.1/lauxlib.h>
    #include <lua5.1/lua.h>
}

int LuaGetTableField(lua_State* L, const char* name, const char* key);

void LuaSetTableField(lua_State* L, const char* name, const char* key, double value);

void LuaSetPaletteColor(lua_State* L, int index, double r, double g, double b);

void LuaSetTableColor(lua_State* L, const char* name, const char* key, int index, double r, double g, double b);
