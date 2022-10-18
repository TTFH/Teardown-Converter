#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <map>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lua.h"
#include "winmm_proxy.h"

using namespace std;

enum MaterialKind {
	None,
	Glass,
	Wood,
	Masonry,
	Plaster,
	Metal,
	HeavyMetal,
	Rock,
	Dirt,
	Foliage,
	Plastic,
	HardMetal,
	HardMasonry,
	Ice,
	Unphysical
};

struct Vector {
	float x, y, z;
};

struct Quat {
	float x, y, z, w;
};

struct Transform {
	Vector pos;
	Quat rot;
};

struct RGBA {
	float r, g, b, a;
};

struct Material {
	uint32_t kind;
	RGBA rgba;
	float reflectivity;
	float shinyness;
	float metalness;
	float emissive;
	uint32_t replaceable;
};

struct Palette {
	uint32_t padding1[3];
	Material materials[256];
	uint8_t tint_table[2 * 4 * 256];
	uint32_t padding2;
};

struct Entity {
	void* unknown;		// 0x00
	uint8_t kind;		// 0x08
	uint8_t flags;		// 0x09
	uint8_t padding[2];
	uint32_t handle;	// 0x0C
	Entity* parent;		// 0x10
	Entity* sibling;	// 0x18
	Entity* child;		// 0x20
}; // size: 0x20

struct Vox {
	uint32_t size[3];		// 0x00
	uint8_t padding1[4];
	uint8_t* voxels;		// 0x10
	void* physics_buffer;
	float scale;			// 0x20
	uint8_t unknown1[48];	// ????
	uint32_t palette;		// 0x54
	uint8_t unknown2[4];	// ????
	int32_t voxel_count;	// 0x5C
}; // size: 0x60

struct Shape {
	Entity self;
	Transform local_tr;			// 0x28
	Vector min_aabb;			// 0x44
	Vector max_aabb;			// 0x50
	uint8_t padding1[4];
	void* dormant;				// 0x60
	void* active;				// 0x68
	uint16_t collision_flags;	// 0x70
	uint8_t collision_layer;	// 0x72
	uint8_t collision_mask;		// 0x73
	float density;				// 0x74
	float strength;				// 0x78
	uint16_t texture_tile;		// 0x7C
	uint16_t blendtexture_tile;	// 0x7E
	float texture_weight;		// 0x80
	float blendtexture_weight;	// 0x84
	Vector starting_wpos;		// 0x88
	uint8_t padding2[4];
	Vox* vox;					// 0x98
	void* joints;				// 0xA0
	float emissive_scale;		// 0xA8
	bool broken;				// 0xAC
	uint8_t padding3[3];
	Transform world_tr;			// 0xB0
	uint8_t padding4[4];
}; // size: 0xD0

class td_string {
	union {
		char* HeapBuffer;
		char StackBuffer[16] = { 0 };
	};
public:
	const char* c_str() const {
		return StackBuffer[15] != '\0' ? HeapBuffer : &StackBuffer[0];
	}
};

template<typename T>
class td_vector {
	uint32_t size = 0;
	uint32_t capacity = 0;
	T* data = nullptr;
public:
	uint32_t getSize() const {
		return size;
	}
	T operator[](uint32_t index) const {
		return data[index];
	}
	T& operator[](uint32_t index) {
		return data[index];
	}
};

struct LuaStateInfo {
	lua_State* state;
};

struct Script {
	Entity self;
	td_string name;				// 0x28
	uint8_t padding1[8];
	float runtime;				// 0x40
	float delta;
	td_string path;				// 0x48
	td_string location;			// 0x58
	uint8_t padding2[56];
	LuaStateInfo* state_info;	// 0xA0
	bool is_loaded;				// 0xA8
	bool has_init;
	bool has_tick;
	bool has_update;
	bool has_draw;
	bool has_command;
	uint8_t padding3[2];
	float runtime2;				// 0xB0
	float update_runtime;		// 0xB4
}; // size: ?

RGBA operator*(const RGBA& color, float scale) {
	return { color.r * scale, color.g * scale, color.b * scale, color.a * scale };
}
RGBA operator+(const RGBA& color1, const RGBA& color2) {
	return { color1.r + color2.r, color1.g + color2.g, color1.b + color2.b, color1.a + color2.a };
}
RGBA operator-(const RGBA& color1, const RGBA& color2) {
	return { color1.r - color2.r, color1.g - color2.g, color1.b - color2.b, color1.a - color2.a };
}

// strength in range [0-4], 0: no paint, 4: fully painted
RGBA paint(RGBA spray_color, RGBA voxel_color, uint8_t strength) {
	RGBA diff = (spray_color - voxel_color) * 0.25;
	return voxel_color + diff * strength;
}

void updateTintTable(RGBA tint, Palette& palette) {
	for (int i = 0; i < 256; i++) {
		for (int strength = 0; strength < 4; strength++) {
			int tint_index = 4 * 256 + strength * 256 + i;
			int index = palette.tint_table[tint_index];
			Material& original_color = palette.materials[i];
			Material& tinted_color = palette.materials[index];
			if (tinted_color.replaceable)
				tinted_color.rgba = paint(tint, original_color.rgba, strength + 1);
		}
	}
}

// TODO: Remove ;-)
uintptr_t FindDMAAddy(uintptr_t addr, vector<unsigned int> offsets) {
	uintptr_t cAddr = addr;
	for (unsigned int i = 0; i < offsets.size(); i++) {
		cAddr = *(uintptr_t*)cAddr;
		if (IsBadReadPtr((uintptr_t*)cAddr, sizeof(cAddr)))
			return 0;
		cAddr += offsets[i];
	}
	return cAddr;
}

void Patch(BYTE* dst, BYTE* src, unsigned int size) {
	DWORD oldProtect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldProtect, &oldProtect);
}

static COLORREF selectedColors[16] = {
	RGB(229, 178,  25), RGB(178, 229,  25), RGB( 25, 229, 178), RGB( 25, 178, 229),
	RGB(178,  25, 229), RGB(229,  25, 178), RGB(229,  25,  28), RGB(151, 151, 151),
	RGB(229, 120,  25), RGB( 35, 229,  25), RGB(234, 176,  92), RGB(169, 129,  68),
	RGB(138,  91,   0), RGB(100,  44,   0), RGB( 56,  39,  32), RGB( 50,  33,  26),
};

RGBA OpenColorPicker(float r, float g, float b) {
	CHOOSECOLOR cc;
	static COLORREF rgbCurrent = RGB(r * 255, g * 255, b * 255);

	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = NULL;
	cc.lpCustColors = (LPDWORD)selectedColors;
	cc.rgbResult = rgbCurrent;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc))
		rgbCurrent = cc.rgbResult;

	RGBA rgba;
	rgba.r = GetRValue(rgbCurrent) / 255.0;
	rgba.g = GetGValue(rgbCurrent) / 255.0;
	rgba.b = GetBValue(rgbCurrent) / 255.0;
	rgba.a = 1.0;
	return rgba;
}

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
	//assert(lua_istable(L, -1))

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

uintptr_t moduleBase = 0;

Shape* GetShape(unsigned int handle) {
	td_vector<Shape*> shapes = *(td_vector<Shape*>*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x158 });
	for (unsigned int i = 0; i < shapes.getSize(); i++)
		if (shapes[i]->self.handle == handle)
			return shapes[i];
	return NULL;
}

int GetPalette(lua_State *L) {
	unsigned int handle = LuaGetTableField(L, "shape", "handle");
	Shape* shape = GetShape(handle);

	if (shape == NULL) {
		printf("[GetPalette] Shape %d not found\n", handle);
		return 0;
	}

	const Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x8, 0x0 });
	Palette selected_palette = palettes[shape->vox->palette];
	for (int i = 0; i < 256; i++) {
		RGBA color = selected_palette.materials[i].rgba;
		LuaSetPaletteColor(L, i != 0 ? i : 256, color.r, color.g, color.b);
	}
	return 0;
}

int GetMaterial(lua_State *L) {
	int index = lua_tointeger(L, 1);
	if (index == 256) index = 0;
	unsigned int handle = LuaGetTableField(L, "shape", "handle");
	Shape* shape = GetShape(handle);

	if (shape == NULL) {
		printf("[GetMaterial] Shape %d not found\n", handle);
		return 0;
	}

	const Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x8, 0x0 });
	Material material = palettes[shape->vox->palette].materials[index];
	LuaSetTableField(L, "indexMaterial", "type", material.kind);
	LuaSetTableField(L, "indexMaterial", "r", material.rgba.r * 255);
	LuaSetTableField(L, "indexMaterial", "g", material.rgba.g * 255);
	LuaSetTableField(L, "indexMaterial", "b", material.rgba.b * 255);
	LuaSetTableField(L, "indexMaterial", "a", material.rgba.a * 255);
	LuaSetTableField(L, "indexMaterial", "reflectivity", material.reflectivity * 100);
	LuaSetTableField(L, "indexMaterial", "shinyness", material.shinyness * 100);
	LuaSetTableField(L, "indexMaterial", "metalness", material.metalness * 100);
	LuaSetTableField(L, "indexMaterial", "emissive", material.emissive * 10);

	LuaSetTableField(L, "shape", "scale", shape->vox->scale * 1000);
	LuaSetTableField(L, "shape", "texture", shape->texture_tile);
	LuaSetTableField(L, "shape", "tex_weight", shape->texture_weight * 100);
	LuaSetTableField(L, "shape", "blend_texture", shape->blendtexture_tile);
	LuaSetTableField(L, "shape", "blend_tex_weight", shape->blendtexture_weight * 100);
	LuaSetTableField(L, "shape", "density", shape->density * 100);
	LuaSetTableField(L, "shape", "strength", shape->strength * 100);
	return 0;
}

int ChangeMaterial(lua_State *L) {
	unsigned int handle = LuaGetTableField(L, "shape", "handle");
	int index = LuaGetTableField(L, "indexMaterial", "index");
	if (index == 256) {
		printf("[ChangeMaterial] Invalid index %d\n", index);
		return 0;
	}
	Shape* shape = GetShape(handle);

	if (shape == NULL) {
		printf("[ChangeMaterial] Shape %d not found\n", handle);
		return 0;
	}

	int type = LuaGetTableField(L, "indexMaterial", "type");
	float red = LuaGetTableField(L, "indexMaterial", "r") / 255.0;
	float green = LuaGetTableField(L, "indexMaterial", "g") / 255.0;
	float blue = LuaGetTableField(L, "indexMaterial", "b") / 255.0;
	float alpha = LuaGetTableField(L, "indexMaterial", "a") / 255.0;
	float reflectivity = LuaGetTableField(L, "indexMaterial", "reflectivity") / 100.0;
	float shinyness = LuaGetTableField(L, "indexMaterial", "shinyness") / 100.0;
	float metalness = LuaGetTableField(L, "indexMaterial", "metalness") / 100.0;
	float emissive = LuaGetTableField(L, "indexMaterial", "emissive") / 10.0;

	Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x8, 0x0 });
	Material& material = palettes[shape->vox->palette].materials[index];
	material.kind = type;
	material.rgba = { red, green, blue, alpha };
	material.reflectivity = reflectivity;
	material.shinyness = shinyness;
	material.metalness = metalness;
	material.emissive = emissive;

	float scale = LuaGetTableField(L, "shape", "scale") / 1000.0;
	int texture_tile = LuaGetTableField(L, "shape", "texture");
	float texture_weight = LuaGetTableField(L, "shape", "tex_weight") / 100.0;
	int blendtexture_tile = LuaGetTableField(L, "shape", "blend_texture");
	float blendtexture_weight = LuaGetTableField(L, "shape", "blend_tex_weight") / 100.0;
	float density = LuaGetTableField(L, "shape", "density") / 100.0;
	float strength = LuaGetTableField(L, "shape", "strength") / 100.0;

	shape->vox->scale = scale;
	shape->texture_tile = texture_tile;
	shape->texture_weight = texture_weight;
	shape->blendtexture_tile = blendtexture_tile;
	shape->blendtexture_weight = blendtexture_weight;
	shape->density = density;
	shape->strength = strength;
	return 0;
}

typedef void (*VoxInitializer) (Vox* vox);
VoxInitializer UpdateVox;
/*
void SetShapeVoxelAtIndex(int handle, int x, int y, int z, int index) {
	Shape* shape = GetShape(handle);
	shape->vox->voxels[x][y][z] = index;
	UpdateVox(shape->vox);
}
*/
int SetShapeVoxelAtIndex(lua_State *L) {
	unsigned int handle = lua_tointeger(L, 1);
	unsigned int x = lua_tointeger(L, 2);
	unsigned int y = lua_tointeger(L, 3);
	unsigned int z = lua_tointeger(L, 4);
	unsigned int index = lua_tointeger(L, 5);

	if (index == 256) index = 0;
	Shape* shape = GetShape(handle);

	if (shape == NULL) {
		printf("[SetShapeVoxelAtIndex] Shape %d not found\n", handle);
		return 0;
	}

	int sizex = shape->vox->size[0];
	int sizey = shape->vox->size[1];
	//int sizez = shape->vox->size[2];
	shape->vox->voxels[x + y * sizex + z * sizex * sizey] = index;

	UpdateVox = (VoxInitializer)FindDMAAddy(moduleBase + 0xFB190, { });
	UpdateVox(shape->vox);
	return 0;
}

int SetRenderDistance(lua_State *L) {
	float dist = lua_tonumber(L, 1);
	Patch((BYTE*)(moduleBase + 0x337620), (BYTE*)&dist, sizeof(float));
	return 0;
}

DWORD WINAPI MainThread(HMODULE hModule) {
#ifdef DEBUGCONSOLE
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);
#endif

	lua_State* L = NULL;
	int color_offset = 0;
	const uint8_t START_INDEX = 209;
	moduleBase = (uintptr_t)GetModuleHandleA("teardown.exe");

	//uintptr_t game = moduleBase + 0x420690;
	//uintptr_t scene = FindDMAAddy(game, { 0x48 });
	//HMODULE OpenGL = GetModuleHandleA("opengl32.dll");
	//wglSwapBuffers = (wglSwapBuffers_t)GetProcAddress(OpenGL, "wglSwapBuffers");

	// 0.9 0.7 0.1 1.0
	const RGBA* spray_color = (RGBA*)FindDMAAddy(moduleBase + 0x34D390, { });
	if (memcmp(spray_color, "\x66\x66\x66\x3F\x33\x33\x33\x3F\xCD\xCC\xCC\x3D\x00\x00\x80\x3F", sizeof(RGBA)) != 0) {
		MessageBoxA(NULL, "The Colored Spraycan DLL mod is not compatible with the version of Teardown you're using, please uninstall it by deleting winmm.dll", "Unsupported version", MB_OK | MB_ICONERROR);
		FreeLibraryAndExitThread(hModule, 0);
		return 0;
	}

	while (true) {
		if (GetAsyncKeyState(VK_F1) & 1) {
			td_vector<Palette> palettes = *(td_vector<Palette>*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x0 });
			printf("Palette count: %d\n", palettes.getSize());

			RGBA new_color = OpenColorPicker(spray_color->r, spray_color->g, spray_color->b);
			Patch((BYTE*)spray_color, (BYTE*)&new_color, sizeof(RGBA));

			for (unsigned int i = 0; i < palettes.getSize(); i++) {
				updateTintTable(new_color, palettes[i]);
				for (int k = 0; k < 16; k++) {
					Material& material = palettes[i].materials[START_INDEX + k];
					float r = GetRValue(selectedColors[k]) / 255.0;
					float g = GetGValue(selectedColors[k]) / 255.0;
					float b = GetBValue(selectedColors[k]) / 255.0;
					material.rgba = { r, g, b, 1.0 };
					material.kind = Masonry;
				}
			}

			// movzx esi, byte ptr [rcx + rsi + 0x2C04];
			const uint8_t* painter_func = (uint8_t*)FindDMAAddy(moduleBase + 0xFEA01, { });
			uint8_t old_painter[8] = { 0x0F, 0xB6, 0xB4, 0x31, 0x04, 0x2C, 0x00, 0x00 };
			Patch((BYTE*)painter_func, old_painter, sizeof(old_painter));

			// je teardown.exe+B37F8
			//const uint8_t* no_update_gpu_texture = (uint8_t*)FindDMAAddy(moduleBase + 0xB3616, { });
			//uint8_t update_gpu_texture[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			//Patch((BYTE*)no_update_gpu_texture, update_gpu_texture, sizeof(update_gpu_texture));
		}

		if (GetAsyncKeyState(VK_F2) & 1) {
			color_offset = (16 + (color_offset - 1) % 16) % 16; // Really?
			const uint8_t* painter_func = (uint8_t*)FindDMAAddy(moduleBase + 0xFEA01, { });
			// mov esi, 0x01; nop; nop; nop;
			uint8_t new_painter[8] = { 0xBE, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };
			new_painter[1] = START_INDEX + color_offset;
			Patch((BYTE*)painter_func, new_painter, sizeof(new_painter));
		}

		if (GetAsyncKeyState(VK_F3) & 1) {
			color_offset = (color_offset + 1) % 16;
			const uint8_t* painter_func = (uint8_t*)FindDMAAddy(moduleBase + 0xFEA01, { });
			uint8_t new_painter[8] = { 0xBE, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };
			new_painter[1] = START_INDEX + color_offset;
			Patch((BYTE*)painter_func, new_painter, sizeof(new_painter));
		}

		if (GetAsyncKeyState(VK_F4) & 1) {
			const unsigned int script_count = *(unsigned int*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x1E8 });
			for (unsigned int i = 0; i < script_count; i++) {
				const Script* script = (Script*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x1F0, 0x8 * i, 0x0 });
				L = script->state_info->state;
				if (strstr(script->name.c_str(), "Colored Spraycan") != NULL || strstr(script->name.c_str(), "2767789311") != NULL) {
					printf("Script %s has state @0x%p\n", script->name.c_str(), (void*)L);
					lua_pushcfunction(L, GetPalette);
					lua_setglobal(L, "GetPalette");
					lua_pushcfunction(L, GetMaterial);
					lua_setglobal(L, "GetMaterial");
					lua_pushcfunction(L, ChangeMaterial);
					lua_setglobal(L, "ChangeMaterial");
					lua_pushcfunction(L, SetShapeVoxelAtIndex);
					lua_setglobal(L, "SetShapeVoxelAtIndex");
				}
				lua_pushcfunction(L, SetRenderDistance);
				lua_setglobal(L, "SetRenderDistance");
			}
		}
		Sleep(34);
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	(void)lpvReserved;
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
