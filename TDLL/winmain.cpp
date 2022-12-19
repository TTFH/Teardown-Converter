#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <map>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "winmm_proxy.h"
#include "teardown_structs.h"

using namespace std;

int color_offset = 0;
bool dithering = true;
uintptr_t moduleBase = 0;
uint8_t* painter_func = NULL;

const uint8_t START_INDEX = 209;
const uint8_t COLOR_COUNT = 16;
const RGBA WHITE = { 1.0, 1.0, 1.0, 1.0 };

typedef void (*VoxInitializer)(Vox* vox);
VoxInitializer UpdateVox;

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

static COLORREF selectedColors[COLOR_COUNT] = {
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

Shape* GetShape(unsigned int handle) {
	td_vector<Shape*> shapes = *(td_vector<Shape*>*)FindDMAAddy(moduleBase + 0x438820, { 0x48, 0x158 });
	for (unsigned int i = 0; i < shapes.getSize(); i++)
		if (shapes[i]->self.handle == handle)
			return shapes[i];
	return NULL;
}

int GetPalette(lua_State* L) {
	unsigned int handle = LuaGetTableField(L, "shape", "handle");
	Shape* shape = GetShape(handle);

	if (shape == NULL) {
		printf("[GetPalette] Shape %d not found\n", handle);
		return 0;
	}

	const Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x438820, { 0xB8, 0x8, 0x0 });
	Palette selected_palette = palettes[shape->vox->palette];
	for (int i = 0; i < 256; i++) {
		RGBA color = selected_palette.materials[i].rgba;
		LuaSetPaletteColor(L, i != 0 ? i : 256, color.r, color.g, color.b);
	}
	return 0;
}

int GetMaterial(lua_State* L) {
	int index = lua_tointeger(L, 1);
	if (index == 256) index = 0;
	unsigned int handle = LuaGetTableField(L, "shape", "handle");
	Shape* shape = GetShape(handle);

	if (shape == NULL) {
		printf("[GetMaterial] Shape %d not found\n", handle);
		return 0;
	}

	const Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x438820, { 0xB8, 0x8, 0x0 });
	Material material = palettes[shape->vox->palette].materials[index];
	LuaSetTableField(L, "indexMaterial", "type", material.kind);
	LuaSetTableField(L, "indexMaterial", "r", material.rgba.r * 255);
	LuaSetTableField(L, "indexMaterial", "g", material.rgba.g * 255);
	LuaSetTableField(L, "indexMaterial", "b", material.rgba.b * 255);
	LuaSetTableField(L, "indexMaterial", "a", material.rgba.a * 255);
	LuaSetTableField(L, "indexMaterial", "reflectivity", material.reflectivity * 100);
	LuaSetTableField(L, "indexMaterial", "shinyness", material.shinyness * 100);
	LuaSetTableField(L, "indexMaterial", "metalness", material.metalness * 100);
	LuaSetTableField(L, "indexMaterial", "emissive", material.emissive * 100);

	LuaSetTableField(L, "shape", "scale", shape->vox->scale * 1000);
	LuaSetTableField(L, "shape", "texture", shape->texture_tile);
	LuaSetTableField(L, "shape", "tex_weight", shape->texture_weight * 100);
	LuaSetTableField(L, "shape", "blend_texture", shape->blendtexture_tile);
	LuaSetTableField(L, "shape", "blend_tex_weight", shape->blendtexture_weight * 100);
	LuaSetTableField(L, "shape", "density", shape->density * 100);
	LuaSetTableField(L, "shape", "strength", shape->strength * 100);
	return 0;
}

int ChangeMaterial(lua_State* L) {
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
	float emissive = LuaGetTableField(L, "indexMaterial", "emissive") / 100.0;

	Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x438820, { 0xB8, 0x8, 0x0 });
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

int SetShapeVoxelAtIndex(lua_State* L) {
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
	shape->vox->voxels[x + y * sizex + z * sizex * sizey] = index;

	// mov [rsp+08], rcx; push rbp; push rbx; push rsi; push rdi;
	// 48 89 4C 24 08 55 53 56 57	0xFB490
	// 40 56 48 81 EC ?? ?? ?? ?? 48 8B F1	0xFCCC0
	UpdateVox = (VoxInitializer)FindDMAAddy(moduleBase + 0xFCCC0, { });
	UpdateVox(shape->vox);
	return 0;
}

int UpdateSpraycanColors(lua_State* L) {
	if (dithering) {
		// movzx esi, byte ptr [rcx + rsi + 0x2C04];
		uint8_t old_painter[8] = { 0x0F, 0xB6, 0xB4, 0x31, 0x04, 0x2C, 0x00, 0x00 };
		Patch((BYTE*)painter_func, old_painter, sizeof(old_painter));
		LuaSetTableField(L, "paint_colors", "selected", 0);
	} else {
		// mov esi, 0x01; nop; nop; nop;
		uint8_t new_painter[8] = { 0xBE, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };
		new_painter[1] = START_INDEX + color_offset;
		Patch((BYTE*)painter_func, new_painter, sizeof(new_painter));
		LuaSetTableField(L, "paint_colors", "selected", color_offset + 1);
	}

	for (int k = 0; k < COLOR_COUNT; k++) {
		float r = GetRValue(selectedColors[k]) / 255.0;
		float g = GetGValue(selectedColors[k]) / 255.0;
		float b = GetBValue(selectedColors[k]) / 255.0;
		LuaSetTableColor(L, "paint_colors", "solid", k + 1, r, g, b);
	}

	const RGBA* spray_color = (RGBA*)FindDMAAddy(moduleBase + 0x35F580, { });
	for (int strength = 1; strength <= 4; strength++) {
		RGBA color = paint(*spray_color, WHITE, strength);
		LuaSetTableColor(L, "paint_colors", "dithering", strength, color.r, color.g, color.b);
	}
	return 0;
}

int ToggleDithering(lua_State* L) {
	(void)L;
	dithering = !dithering;
	return 0;
}

int NextSpraycanColor(lua_State* L) {
	(void)L;
	dithering = false;
	color_offset = (color_offset + 1) % COLOR_COUNT;
	return 0;
}

int PrevSpraycanColor(lua_State* L) {
	(void)L;
	dithering = false;
	color_offset = (COLOR_COUNT + (color_offset - 1) % COLOR_COUNT) % COLOR_COUNT;
	return 0;
}

int SetRenderDistance(lua_State* L) {
	float dist = lua_tonumber(L, 1);
	Patch((BYTE*)(moduleBase + 0x3496B0), (BYTE*)&dist, sizeof(float));
	return 0;
}

int SetDevMenuVisibility(lua_State* L) {
	bool& toggle = *(bool*)FindDMAAddy(moduleBase + 0x438820, { 0x90, 0x0 });
	toggle = lua_toboolean(L, 1);
	return 0;
}

int GetDllVersion(lua_State* L) {
	lua_pushstring(L, "v1.3.0_b01");
	return 1;
}

DWORD WINAPI MainThread(HMODULE hModule) {
#ifdef DEBUGCONSOLE
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);
#endif
	moduleBase = (uintptr_t)GetModuleHandleA("teardown.exe");
	painter_func = (uint8_t*)FindDMAAddy(moduleBase + 0x100801, { });

	// f:0.9 f:0.7 f:0.1 f:1.0
	const RGBA* spray_color = (RGBA*)FindDMAAddy(moduleBase + 0x35F580, { });
	if (memcmp(spray_color, "\x66\x66\x66\x3F\x33\x33\x33\x3F\xCD\xCC\xCC\x3D\x00\x00\x80\x3F", sizeof(RGBA)) != 0) {
		MessageBoxA(NULL, "The Colored Spraycan DLL mod is not compatible with the version of Teardown you're using, please uninstall it by deleting winmm.dll", "Unsupported version", MB_OK | MB_ICONERROR);
		FreeLibraryAndExitThread(hModule, 0);
		return 0;
	}

	while (true) {
		if (GetAsyncKeyState(VK_F1) & 1) {
			bool init = false;
			// TODO: use td_vector<>
			const unsigned int script_count = *(unsigned int*)FindDMAAddy(moduleBase + 0x438820, { 0x48, 0x1E8 });
			for (unsigned int i = 0; i < script_count; i++) {
				const Script* script = (Script*)FindDMAAddy(moduleBase + 0x438820, { 0x48, 0x1F0, 0x8 * i, 0x0 });
				lua_State* L = script->state_info->state;
				if (!init && (strstr(script->name.c_str(), "Colored Spraycan") != NULL || strstr(script->name.c_str(), "2767789311") != NULL)) {
					init = true; // In case there are multiples copies of the mod
					printf("Script %s has state @0x%p\n", script->name.c_str(), (void*)L);
					lua_pushcfunction(L, GetPalette);
					lua_setglobal(L, "GetPalette");
					lua_pushcfunction(L, GetMaterial);
					lua_setglobal(L, "GetMaterial");
					lua_pushcfunction(L, ChangeMaterial);
					lua_setglobal(L, "ChangeMaterial");
					lua_pushcfunction(L, SetShapeVoxelAtIndex);
					lua_setglobal(L, "SetShapeVoxelAtIndex");

					lua_pushcfunction(L, UpdateSpraycanColors);
					lua_setglobal(L, "UpdateSpraycanColors");
					lua_pushcfunction(L, ToggleDithering);
					lua_setglobal(L, "ToggleDithering");
					lua_pushcfunction(L, NextSpraycanColor);
					lua_setglobal(L, "NextSpraycanColor");
					lua_pushcfunction(L, PrevSpraycanColor);
					lua_setglobal(L, "PrevSpraycanColor");
				}

				// Public functions
				lua_pushcfunction(L, SetRenderDistance);
				lua_setglobal(L, "SetRenderDistance");
				lua_pushcfunction(L, SetDevMenuVisibility);
				lua_setglobal(L, "SetDevMenuVisibility");
				lua_pushcfunction(L, GetDllVersion);
				lua_setglobal(L, "GetDllVersion");
				// TODO: EnableShapeCollision
			}
		}

		if (GetAsyncKeyState(VK_F2) & 1) {
			td_vector<Palette> palettes = *(td_vector<Palette>*)FindDMAAddy(moduleBase + 0x438820, { 0xB8, 0x0 });
			RGBA new_color = OpenColorPicker(spray_color->r, spray_color->g, spray_color->b);
			Patch((BYTE*)spray_color, (BYTE*)&new_color, sizeof(RGBA));

			for (unsigned int i = 0; i < palettes.getSize(); i++) {
				updateTintTable(new_color, palettes[i]);
				for (int k = 0; k < COLOR_COUNT; k++) {
					Material& material = palettes[i].materials[START_INDEX + k];
					float r = GetRValue(selectedColors[k]) / 255.0;
					float g = GetGValue(selectedColors[k]) / 255.0;
					float b = GetBValue(selectedColors[k]) / 255.0;
					material.rgba = { r, g, b, 1.0 };
					material.kind = Masonry;
				}
			}
		}
		Sleep(34);
	}
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	(void)lpvReserved;
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
