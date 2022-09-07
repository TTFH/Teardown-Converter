// g++ -Wall -Werror winmain.cpp -o winmm.dll -s -shared -lcomdlg32 -static

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>

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
	uint32_t replacable;
};

struct Palette {
	Material materials[256];
	uint8_t tint_table[2 * 4 * 256];
	uint32_t padding[4];
};

// TODO: use alpha
RGBA operator*(const RGBA& color, float scale) {
	return { color.r * scale, color.g * scale, color.b * scale, 1 };
}
RGBA operator+(const RGBA& color1, const RGBA& color2) {
	return { color1.r + color2.r, color1.g + color2.g, color1.b + color2.b, 1 };
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
			int tint_index = 4 * 256 + 256 * strength + i;
			int index = palette.tint_table[tint_index];
			Material& original_color = palette.materials[i];
			Material& tinted_color = palette.materials[index];
			if (tinted_color.replacable)
				tinted_color.rgba = paint(tint, original_color.rgba, strength + 1);
		}
	}
}

uintptr_t FindDMAAddy(uintptr_t addr, std::vector<unsigned int> offsets) {
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

RGBA OpenColorPicker(float r, float g, float b) {
	CHOOSECOLOR cc;
	static COLORREF acrCustClr[16];
	static COLORREF rgbCurrent = RGB(r * 255, g * 255, b * 255);

	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = NULL;
	cc.lpCustColors = (LPDWORD)acrCustClr;
	cc.rgbResult = rgbCurrent;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc))
		rgbCurrent = cc.rgbResult;

	RGBA rgba;
	rgba.r = GetRValue(rgbCurrent) / 255.0f;
	rgba.g = GetGValue(rgbCurrent) / 255.0f;
	rgba.b = GetBValue(rgbCurrent) / 255.0f;
	rgba.a = 1.0f;
	return rgba;
}

DWORD WINAPI MainThread(HMODULE hModule) {
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);
	uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("teardown.exe");

	while (true) {
		if (GetAsyncKeyState(VK_F1) & 1) {
			const RGBA* spray_color = (RGBA*)FindDMAAddy(moduleBase + 0x34D390, {});
			// Removing the next line make the linker unhappy
			printf("Spray Color: %g %g %g %g\n", spray_color->r, spray_color->g, spray_color->b, spray_color->a);
			RGBA new_color = OpenColorPicker(spray_color->r, spray_color->g, spray_color->b);
			Patch((BYTE*)spray_color, (BYTE*)&new_color, sizeof(RGBA));

			const int palette_count = *(int*)FindDMAAddy(moduleBase + 0x00420690, {0xB8, 0x0});
			Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x00420690, {0xB8, 0x8, 0xC});
			printf("Palette Count: %d\n", palette_count);
			printf("Palettes at: %p\n", palettes);
			for (int i = 0; i < palette_count; i++)
				updateTintTable(new_color, palettes[i]);
		}
		Sleep(10);
	}
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
