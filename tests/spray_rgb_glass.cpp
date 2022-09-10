#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <vector>

struct RGBA {
	float r, g, b, a;
};

struct Material;
struct Palette;

uintptr_t FindDMAAddy(uintptr_t addr, std::vector<unsigned int> offsets);
void Patch(BYTE* dst, BYTE* src, unsigned int size);

// H [0, 360] S, V and A [0.0, 1.0].
RGBA FromHSV(float h, float s, float v, float a) {
	int i = (int)floor(h / 60.0f) % 6;
	float f = h / 60.0f - floor(h / 60.0f);
	float p = v * (float)(1 - s);
	float q = v * (float)(1 - s * f);
	float t = v * (float)(1 - (1 - f) * s);

	switch (i) {
		case 0: return { v, t, p, a };
		case 1: return { q, v, p, a };
		case 2: return { p, v, t, a };
		case 3: return { p, q, v, a };
		case 4: return { t, p, v, a };
		case 5: return { v, p, q, a };
	}
	return { 0, 0, 0, 1 };
}

DWORD WINAPI RGBThread(HMODULE hModule) {
	//AllocConsole();
	//FILE* stream;
	//freopen_s(&stream, "CONOUT$", "w", stdout);

	int k = 0;
	bool close = false;
	bool enabled = false;
	const uint8_t PAINT_INDEX = 209;
	uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("teardown.exe");

	while (!close) {
		if (GetAsyncKeyState(VK_F1) & 1) {
			const uint8_t* no_update_gpu_texture = (uint8_t*)FindDMAAddy(moduleBase + 0xB3616, { });
			uint8_t update_gpu_texture[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			Patch((BYTE*)no_update_gpu_texture, update_gpu_texture, sizeof(update_gpu_texture));

			const uint8_t* painter_func = (uint8_t*)FindDMAAddy(moduleBase + 0xFEA01, { });
			uint8_t new_painter[8] = { 0xBE, PAINT_INDEX, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };
			Patch((BYTE*)painter_func, new_painter, sizeof(new_painter));

			enabled = true;
		}
		if (enabled) {
			const int palette_count = *(int*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x0 });
			Palette* palettes = (Palette*)FindDMAAddy(moduleBase + 0x420690, { 0xB8, 0x8, 0xC });
			for (int i = 0; i < palette_count; i++) {
				Material& material = palettes[i].materials[PAINT_INDEX];
				material.rgba = FromHSV(k, 0.9, 0.9, 0.9);
				material.kind = Glass;
			}
			k = (k + 8) % 360;
			Sleep(100);
		}
		if (GetAsyncKeyState(VK_F2) & 1)
			close = true;
		Sleep(32);
	}

	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}
