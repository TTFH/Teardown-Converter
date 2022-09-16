// g++ -Wall -Wextra -Werror -Wpedantic winmain.cpp -o winmm.dll -s -shared -lcomdlg32 -static -llua5.1 -DDEBUG

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <map>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern "C" {
    #include <lua5.1/lualib.h>
    #include <lua5.1/lauxlib.h>
    #include <lua5.1/lua.h>
}

using namespace std;

#define EXPORT(exp_name, target_name) asm(".section .drectve\n\t.ascii \" -export:" #exp_name "= c:/windows/system32/" #target_name "\"")
#if TRUE
EXPORT(CloseDriver, winmm.CloseDriver);
EXPORT(DefDriverProc, winmm.DefDriverProc);
EXPORT(DriverCallback, winmm.DriverCallback);
EXPORT(DrvGetModuleHandle, winmm.DrvGetModuleHandle);
EXPORT(GetDriverModuleHandle, winmm.GetDriverModuleHandle);
EXPORT(OpenDriver, winmm.OpenDriver);
EXPORT(PlaySound, winmm.PlaySound);
EXPORT(PlaySoundA, winmm.PlaySoundA);
EXPORT(PlaySoundW, winmm.PlaySoundW);
EXPORT(SendDriverMessage, winmm.SendDriverMessage);
EXPORT(WOWAppExit, winmm.WOWAppExit);
EXPORT(auxGetDevCapsA, winmm.auxGetDevCapsA);
EXPORT(auxGetDevCapsW, winmm.auxGetDevCapsW);
EXPORT(auxGetNumDevs, winmm.auxGetNumDevs);
EXPORT(auxGetVolume, winmm.auxGetVolume);
EXPORT(auxOutMessage, winmm.auxOutMessage);
EXPORT(auxSetVolume, winmm.auxSetVolume);
EXPORT(joyConfigChanged, winmm.joyConfigChanged);
EXPORT(joyGetDevCapsA, winmm.joyGetDevCapsA);
EXPORT(joyGetDevCapsW, winmm.joyGetDevCapsW);
EXPORT(joyGetNumDevs, winmm.joyGetNumDevs);
EXPORT(joyGetPos, winmm.joyGetPos);
EXPORT(joyGetPosEx, winmm.joyGetPosEx);
EXPORT(joyGetThreshold, winmm.joyGetThreshold);
EXPORT(joyReleaseCapture, winmm.joyReleaseCapture);
EXPORT(joySetCapture, winmm.joySetCapture);
EXPORT(joySetThreshold, winmm.joySetThreshold);
EXPORT(mciDriverNotify, winmm.mciDriverNotify);
EXPORT(mciDriverYield, winmm.mciDriverYield);
EXPORT(mciExecute, winmm.mciExecute);
EXPORT(mciFreeCommandResource, winmm.mciFreeCommandResource);
EXPORT(mciGetCreatorTask, winmm.mciGetCreatorTask);
EXPORT(mciGetDeviceIDA, winmm.mciGetDeviceIDA);
EXPORT(mciGetDeviceIDFromElementIDA, winmm.mciGetDeviceIDFromElementIDA);
EXPORT(mciGetDeviceIDFromElementIDW, winmm.mciGetDeviceIDFromElementIDW);
EXPORT(mciGetDeviceIDW, winmm.mciGetDeviceIDW);
EXPORT(mciGetDriverData, winmm.mciGetDriverData);
EXPORT(mciGetErrorStringA, winmm.mciGetErrorStringA);
EXPORT(mciGetErrorStringW, winmm.mciGetErrorStringW);
EXPORT(mciGetYieldProc, winmm.mciGetYieldProc);
EXPORT(mciLoadCommandResource, winmm.mciLoadCommandResource);
EXPORT(mciSendCommandA, winmm.mciSendCommandA);
EXPORT(mciSendCommandW, winmm.mciSendCommandW);
EXPORT(mciSendStringA, winmm.mciSendStringA);
EXPORT(mciSendStringW, winmm.mciSendStringW);
EXPORT(mciSetDriverData, winmm.mciSetDriverData);
EXPORT(mciSetYieldProc, winmm.mciSetYieldProc);
EXPORT(midiConnect, winmm.midiConnect);
EXPORT(midiDisconnect, winmm.midiDisconnect);
EXPORT(midiInAddBuffer, winmm.midiInAddBuffer);
EXPORT(midiInClose, winmm.midiInClose);
EXPORT(midiInGetDevCapsA, winmm.midiInGetDevCapsA);
EXPORT(midiInGetDevCapsW, winmm.midiInGetDevCapsW);
EXPORT(midiInGetErrorTextA, winmm.midiInGetErrorTextA);
EXPORT(midiInGetErrorTextW, winmm.midiInGetErrorTextW);
EXPORT(midiInGetID, winmm.midiInGetID);
EXPORT(midiInGetNumDevs, winmm.midiInGetNumDevs);
EXPORT(midiInMessage, winmm.midiInMessage);
EXPORT(midiInOpen, winmm.midiInOpen);
EXPORT(midiInPrepareHeader, winmm.midiInPrepareHeader);
EXPORT(midiInReset, winmm.midiInReset);
EXPORT(midiInStart, winmm.midiInStart);
EXPORT(midiInStop, winmm.midiInStop);
EXPORT(midiInUnprepareHeader, winmm.midiInUnprepareHeader);
EXPORT(midiOutCacheDrumPatches, winmm.midiOutCacheDrumPatches);
EXPORT(midiOutCachePatches, winmm.midiOutCachePatches);
EXPORT(midiOutClose, winmm.midiOutClose);
EXPORT(midiOutGetDevCapsA, winmm.midiOutGetDevCapsA);
EXPORT(midiOutGetDevCapsW, winmm.midiOutGetDevCapsW);
EXPORT(midiOutGetErrorTextA, winmm.midiOutGetErrorTextA);
EXPORT(midiOutGetErrorTextW, winmm.midiOutGetErrorTextW);
EXPORT(midiOutGetID, winmm.midiOutGetID);
EXPORT(midiOutGetNumDevs, winmm.midiOutGetNumDevs);
EXPORT(midiOutGetVolume, winmm.midiOutGetVolume);
EXPORT(midiOutLongMsg, winmm.midiOutLongMsg);
EXPORT(midiOutMessage, winmm.midiOutMessage);
EXPORT(midiOutOpen, winmm.midiOutOpen);
EXPORT(midiOutPrepareHeader, winmm.midiOutPrepareHeader);
EXPORT(midiOutReset, winmm.midiOutReset);
EXPORT(midiOutSetVolume, winmm.midiOutSetVolume);
EXPORT(midiOutShortMsg, winmm.midiOutShortMsg);
EXPORT(midiOutUnprepareHeader, winmm.midiOutUnprepareHeader);
EXPORT(midiStreamClose, winmm.midiStreamClose);
EXPORT(midiStreamOpen, winmm.midiStreamOpen);
EXPORT(midiStreamOut, winmm.midiStreamOut);
EXPORT(midiStreamPause, winmm.midiStreamPause);
EXPORT(midiStreamPosition, winmm.midiStreamPosition);
EXPORT(midiStreamProperty, winmm.midiStreamProperty);
EXPORT(midiStreamRestart, winmm.midiStreamRestart);
EXPORT(midiStreamStop, winmm.midiStreamStop);
EXPORT(mixerClose, winmm.mixerClose);
EXPORT(mixerGetControlDetailsA, winmm.mixerGetControlDetailsA);
EXPORT(mixerGetControlDetailsW, winmm.mixerGetControlDetailsW);
EXPORT(mixerGetDevCapsA, winmm.mixerGetDevCapsA);
EXPORT(mixerGetDevCapsW, winmm.mixerGetDevCapsW);
EXPORT(mixerGetID, winmm.mixerGetID);
EXPORT(mixerGetLineControlsA, winmm.mixerGetLineControlsA);
EXPORT(mixerGetLineControlsW, winmm.mixerGetLineControlsW);
EXPORT(mixerGetLineInfoA, winmm.mixerGetLineInfoA);
EXPORT(mixerGetLineInfoW, winmm.mixerGetLineInfoW);
EXPORT(mixerGetNumDevs, winmm.mixerGetNumDevs);
EXPORT(mixerMessage, winmm.mixerMessage);
EXPORT(mixerOpen, winmm.mixerOpen);
EXPORT(mixerSetControlDetails, winmm.mixerSetControlDetails);
EXPORT(mmDrvInstall, winmm.mmDrvInstall);
EXPORT(mmGetCurrentTask, winmm.mmGetCurrentTask);
EXPORT(mmTaskBlock, winmm.mmTaskBlock);
EXPORT(mmTaskCreate, winmm.mmTaskCreate);
EXPORT(mmTaskSignal, winmm.mmTaskSignal);
EXPORT(mmTaskYield, winmm.mmTaskYield);
EXPORT(mmioAdvance, winmm.mmioAdvance);
EXPORT(mmioAscend, winmm.mmioAscend);
EXPORT(mmioClose, winmm.mmioClose);
EXPORT(mmioCreateChunk, winmm.mmioCreateChunk);
EXPORT(mmioDescend, winmm.mmioDescend);
EXPORT(mmioFlush, winmm.mmioFlush);
EXPORT(mmioGetInfo, winmm.mmioGetInfo);
EXPORT(mmioInstallIOProcA, winmm.mmioInstallIOProcA);
EXPORT(mmioInstallIOProcW, winmm.mmioInstallIOProcW);
EXPORT(mmioOpenA, winmm.mmioOpenA);
EXPORT(mmioOpenW, winmm.mmioOpenW);
EXPORT(mmioRead, winmm.mmioRead);
EXPORT(mmioRenameA, winmm.mmioRenameA);
EXPORT(mmioRenameW, winmm.mmioRenameW);
EXPORT(mmioSeek, winmm.mmioSeek);
EXPORT(mmioSendMessage, winmm.mmioSendMessage);
EXPORT(mmioSetBuffer, winmm.mmioSetBuffer);
EXPORT(mmioSetInfo, winmm.mmioSetInfo);
EXPORT(mmioStringToFOURCCA, winmm.mmioStringToFOURCCA);
EXPORT(mmioStringToFOURCCW, winmm.mmioStringToFOURCCW);
EXPORT(mmioWrite, winmm.mmioWrite);
EXPORT(mmsystemGetVersion, winmm.mmsystemGetVersion);
EXPORT(sndPlaySoundA, winmm.sndPlaySoundA);
EXPORT(sndPlaySoundW, winmm.sndPlaySoundW);
EXPORT(timeBeginPeriod, winmm.timeBeginPeriod);
EXPORT(timeEndPeriod, winmm.timeEndPeriod);
EXPORT(timeGetDevCaps, winmm.timeGetDevCaps);
EXPORT(timeGetSystemTime, winmm.timeGetSystemTime);
EXPORT(timeGetTime, winmm.timeGetTime);
EXPORT(timeKillEvent, winmm.timeKillEvent);
EXPORT(timeSetEvent, winmm.timeSetEvent);
EXPORT(waveInAddBuffer, winmm.waveInAddBuffer);
EXPORT(waveInClose, winmm.waveInClose);
EXPORT(waveInGetDevCapsA, winmm.waveInGetDevCapsA);
EXPORT(waveInGetDevCapsW, winmm.waveInGetDevCapsW);
EXPORT(waveInGetErrorTextA, winmm.waveInGetErrorTextA);
EXPORT(waveInGetErrorTextW, winmm.waveInGetErrorTextW);
EXPORT(waveInGetID, winmm.waveInGetID);
EXPORT(waveInGetNumDevs, winmm.waveInGetNumDevs);
EXPORT(waveInGetPosition, winmm.waveInGetPosition);
EXPORT(waveInMessage, winmm.waveInMessage);
EXPORT(waveInOpen, winmm.waveInOpen);
EXPORT(waveInPrepareHeader, winmm.waveInPrepareHeader);
EXPORT(waveInReset, winmm.waveInReset);
EXPORT(waveInStart, winmm.waveInStart);
EXPORT(waveInStop, winmm.waveInStop);
EXPORT(waveInUnprepareHeader, winmm.waveInUnprepareHeader);
EXPORT(waveOutBreakLoop, winmm.waveOutBreakLoop);
EXPORT(waveOutClose, winmm.waveOutClose);
EXPORT(waveOutGetDevCapsA, winmm.waveOutGetDevCapsA);
EXPORT(waveOutGetDevCapsW, winmm.waveOutGetDevCapsW);
EXPORT(waveOutGetErrorTextA, winmm.waveOutGetErrorTextA);
EXPORT(waveOutGetErrorTextW, winmm.waveOutGetErrorTextW);
EXPORT(waveOutGetID, winmm.waveOutGetID);
EXPORT(waveOutGetNumDevs, winmm.waveOutGetNumDevs);
EXPORT(waveOutGetPitch, winmm.waveOutGetPitch);
EXPORT(waveOutGetPlaybackRate, winmm.waveOutGetPlaybackRate);
EXPORT(waveOutGetPosition, winmm.waveOutGetPosition);
EXPORT(waveOutGetVolume, winmm.waveOutGetVolume);
EXPORT(waveOutMessage, winmm.waveOutMessage);
EXPORT(waveOutOpen, winmm.waveOutOpen);
EXPORT(waveOutPause, winmm.waveOutPause);
EXPORT(waveOutPrepareHeader, winmm.waveOutPrepareHeader);
EXPORT(waveOutReset, winmm.waveOutReset);
EXPORT(waveOutRestart, winmm.waveOutRestart);
EXPORT(waveOutSetPitch, winmm.waveOutSetPitch);
EXPORT(waveOutSetPlaybackRate, winmm.waveOutSetPlaybackRate);
EXPORT(waveOutSetVolume, winmm.waveOutSetVolume);
EXPORT(waveOutUnprepareHeader, winmm.waveOutUnprepareHeader);
EXPORT(waveOutWrite, winmm.waveOutWrite);
#endif

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
	Material materials[256];
	uint8_t tint_table[2 * 4 * 256];
	uint32_t padding[4];
};

struct Vox {
	uint32_t size[3];
	uint8_t padding_1[4];
	void* material_buffer;
	void* physics_buffer;
	float scale;
	uint8_t padding_2[48];
	uint32_t palette;
	uint8_t padding_3[4];
	int32_t voxel_count;
}; // size: 0x0060

RGBA operator*(const RGBA& color, float scale) {
	return { color.r * scale, color.g * scale, color.b * scale, color.a * scale };
}
RGBA operator+(const RGBA& color1, const RGBA& color2) {
	return { color1.r + color2.r, color1.g + color2.g, color1.b + color2.b, color1.a + color2.a };
}
RGBA operator-(const RGBA& color1, const RGBA& color2) {
	return { color1.r - color2.r, color1.g - color2.g, color1.b - color2.b, 1 };
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

int LuaGetInt(lua_State* L, const char* name) {
	int result = 0;
	lua_getglobal(L, name);
	if (lua_isnumber(L, -1))
		result = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return result;
}

void LuaSetInt(lua_State* L, const char* name, double value) {
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
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
	if (!lua_istable(L, -1))
		printf("Palette is missing index %d\n", index);

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
map<int, int> shape_palette;

int UpdatePalette(lua_State *L) {
	int g_shape = lua_tointeger(L, 1);
	printf("Get palette for shape %d\n", g_shape);
	int palette_id = shape_palette[g_shape];
	LuaSetInt(L, "g_palette_id", palette_id);

	const Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x8, 0xC });
	Palette selected_palette = palettes[palette_id];
	for (int i = 0; i < 255; i++) {
		RGBA color = selected_palette.materials[i].rgba;
		LuaSetPaletteColor(L, i != 0 ? i : 256, color.r, color.g, color.b);
	}
	return 0;
}

int UpdateMaterial(lua_State *L) {
	int g_shape = lua_tointeger(L, 1);
	int index = lua_tointeger(L, 2);
	printf("Get material for shape %d and index %d\n", g_shape, index);

	const Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x8, 0xC });
	int palette_id = shape_palette[g_shape];
	Palette selected_palette = palettes[palette_id];
	Material material = selected_palette.materials[index];
	LuaSetTableField(L, "indexMaterial", "type", material.kind);
	LuaSetTableField(L, "indexMaterial", "r", material.rgba.r * 255);
	LuaSetTableField(L, "indexMaterial", "g", material.rgba.g * 255);
	LuaSetTableField(L, "indexMaterial", "b", material.rgba.b * 255);
	LuaSetTableField(L, "indexMaterial", "a", material.rgba.a * 255);
	LuaSetTableField(L, "indexMaterial", "reflectivity", material.reflectivity * 100);
	LuaSetTableField(L, "indexMaterial", "shinyness", material.shinyness * 100);
	LuaSetTableField(L, "indexMaterial", "metalness", material.metalness * 100);
	LuaSetTableField(L, "indexMaterial", "emissive", material.emissive * 10);
	lua_getglobal(L, "indexMaterial");
	lua_pushboolean(L, material.replaceable);
	lua_setfield(L, -2, "replaceable");
	lua_pop(L, 1);
	return 0;
}

int ChangeMaterial(lua_State *L) {
	int g_shape = lua_tointeger(L, 1);
	int index = lua_tointeger(L, 2);
	printf("CHANGE material for shape %d and index %d\n", g_shape, index);
	if (index == 256) {
		printf("Invalid index 256\n");
		return 0;
	}

	int type = lua_tointeger(L, 3);
	float red = lua_tonumber(L, 4) / 255.0;
	float green = lua_tonumber(L, 5) / 255.0;
	float blue = lua_tonumber(L, 6) / 255.0;
	float alpha = lua_tonumber(L, 7) / 255.0;
	float reflectivity = lua_tonumber(L, 8) / 100.0;
	float shinyness = lua_tonumber(L, 9) / 100.0;
	float metalness = lua_tonumber(L, 10) / 100.0;
	float emissive = lua_tonumber(L, 11) / 10.0;

	int palette_id = shape_palette[g_shape];
	Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x8, 0xC });
	Material& material = palettes[palette_id].materials[index];
	material.kind = type;
	material.rgba = { red, green, blue, alpha };
	material.reflectivity = reflectivity;
	material.shinyness = shinyness;
	material.metalness = metalness;
	material.emissive = emissive;
	return 0;
}

DWORD WINAPI MainThread(HMODULE hModule) {
#ifdef DEBUG
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);
#endif

	lua_State* L = NULL;
	int color_offset = 0;
	const uint8_t START_INDEX = 209;
	moduleBase = (uintptr_t)GetModuleHandleA("teardown.exe");

	// 0.9 0.7 0.1 1.0
	const RGBA* spray_color = (RGBA*)FindDMAAddy(moduleBase + 0x34D390, { });
	if (memcmp(spray_color, "\x66\x66\x66\x3F\x33\x33\x33\x3F\xCD\xCC\xCC\x3D\x00\x00\x80\x3F", sizeof(RGBA)) != 0) {
		MessageBoxA(NULL, "This MOD is not compatible with the version of Teardown you're using, please uninstall it by deleting winmm.dll", "Unsupported version", MB_OK | MB_ICONERROR);
		FreeLibraryAndExitThread(hModule, 0);
		return 0;
	}

	while (true) {
		if (GetAsyncKeyState(VK_F1) & 1) {
			const int palette_count = *(int*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x0 });
			Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x8, 0xC });
			// Removing this comment will cause the linker to crash ¯\_(ツ)_/¯
			printf("Palette count: %d\n", palette_count);

			RGBA new_color = OpenColorPicker(spray_color->r, spray_color->g, spray_color->b);
			Patch((BYTE*)spray_color, (BYTE*)&new_color, sizeof(RGBA));

			for (int i = 0; i < palette_count; i++) {
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

			// je teardown.exe+B37F8
			//const uint8_t* no_update_gpu_texture = (uint8_t*)FindDMAAddy(moduleBase + 0xB3616, { });
			//uint8_t update_gpu_texture[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			//Patch((BYTE*)no_update_gpu_texture, update_gpu_texture, sizeof(update_gpu_texture));
		}

		if (GetAsyncKeyState(VK_F2) & 1) {
			color_offset = (16 + (color_offset - 1) % 16) % 16; // Really?
			// movzx esi, byte ptr [rcx + rsi + 0x2C04];
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
			bool init = false;
			shape_palette.clear();
			const unsigned int shapes_count = *(unsigned int*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x158 });
			printf("Shapes count: %d\n", shapes_count);
			for (unsigned int i = 0; i < shapes_count; i++) {
				const int shape_handle = *(int*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x160, 0x8 * i, 0xC });
				const Vox* shape_data = (Vox*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x160, 0x8 * i, 0x98, 0x0 });
				//printf("Shape %d of size (%d, %d, %d) and %d voxels use palette %d\n", shape_handle,
				//	shape_data->size[0], shape_data->size[1], shape_data->size[2], shape_data->voxel_count, shape_data->palette);
				shape_palette[shape_handle] = shape_data->palette;
			}
			const unsigned int script_count = *(unsigned int*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x1E8 });
			printf("Script count: %d\n", script_count);
			for (unsigned int i = 0; i < script_count; i++) {
				const char* script_path = (char*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x1F0, 0x8 * i, 0x28, 0x0 });
				if (!init && (strstr(script_path, "Colored Spraycan") != NULL || strstr(script_path, "2767789311") != NULL)) {
					init = true;
					L = (lua_State*)FindDMAAddy(moduleBase + 0x420690, { 0x48, 0x1F0, 0x8 * i, 0xA0, 0x0, 0x0 });
					printf("Script %s with state %p\n", script_path, (void*)L);
					lua_pushcfunction(L, UpdatePalette);
					lua_setglobal(L, "UpdatePalette");
					lua_pushcfunction(L, UpdateMaterial);
					lua_setglobal(L, "UpdateMaterial");
					lua_pushcfunction(L, ChangeMaterial);
					lua_setglobal(L, "ChangeMaterial");
				}
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
