bool Detour32(BYTE* src, const BYTE* dst, const uintptr_t len) {
	if (len < 5) return false;
	DWORD curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);
	uintptr_t relativeAddress = dst - src - 5;
	*src = 0xE9;
	*((uintptr_t*)(src + 0x1)) = relativeAddress;
	VirtualProtect(src, len, curProtection, &curProtection);
	return true;
}

BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len) {
	if (len < 5) return 0;
	BYTE* gateway = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(gateway, src, len);
	uintptr_t gatewayRelativeAddress = src - gateway - 5;
	*(gateway + len) = 0xE9;
	*((uintptr_t*)(gateway + len + 0x1)) = gatewayRelativeAddress;
	Detour32(src, dst, len);
	return gateway;
}

class Hook {
private:
	BYTE* src = nullptr;
	BYTE* dst = nullptr;
	uintptr_t len = 0;
	BYTE* PtrToGatewayFnPtr = nullptr;
	BYTE* originalBytes[16] = { 0 };
public:
	Hook(const char* exportName, const char* moduleName, BYTE* dst, BYTE* PtrToGatewayFnPtr, uintptr_t len) {
		HMODULE hModule = GetModuleHandleA(moduleName);
		this->src = (BYTE*)GetProcAddress(hModule, exportName);
		this->dst = dst;
		this->len = len;
		this->PtrToGatewayFnPtr = PtrToGatewayFnPtr;
	}
	void Enable() {
		memcpy(originalBytes, src, len);
		*(uintptr_t*)PtrToGatewayFnPtr = (uintptr_t)TrampHook32(src, dst, len);
	}
};

typedef BOOL(__stdcall* twglSwapBuffers) (HDC hDc);
twglSwapBuffers wglSwapBuffersGateway;
/*
void Draw() {
	HDC currentHDC = wglGetCurrentDC();
	printf("HDC: %p\n", (void*)currentHDC);
}
*/
BOOL __stdcall hkwglSwapBuffers(HDC hDc) {
	printf("HDC: %p\n", (void*)hDc);
	return wglSwapBuffersGateway(hDc);
}

typedef BOOL(__stdcall* wglSwapBuffers_t)(HDC hDc);
wglSwapBuffers_t wglSwapBuffers;

Hook SwapBuffersHook("wglSwapBuffers", "opengl32.dll", (BYTE*)hkwglSwapBuffers, (BYTE*)&wglSwapBuffersGateway, 5);
if (GetAsyncKeyState(VK_F5) & 1)
	SwapBuffersHook.Enable();

HMODULE OpenGL = GetModuleHandleA("opengl32.dll");
wglSwapBuffers = (wglSwapBuffers_t)GetProcAddress(OpenGL, "wglSwapBuffers");


	bool scaled = false;
	unsigned int prev_state = Playing; // Default value for some reason
	while (true) {
		// grouped scan 4:1920 4:1080 4:4
		const Game* game = (Game*)FindDMAAddy(moduleBase + 0x438820, { 0x0 });
		if (game != NULL && game->state != prev_state) {
			prev_state = game->state;
			printf("Game state: %d\n", game->state);
			if (game->state == Playing) {
				printf("DLL Injected!\n");
				Sleep(5000); // Wait for the level to load
				// Init code goes here
			}
		}
		if (!scaled && GetAsyncKeyState(VK_F3) & 1) {
			scaled = true;
			float scale = 0.5f
			td_vector<Body*> bodies = *(td_vector<Body*>*)FindDMAAddy(moduleBase + 0x438820, { 0x48, 0x148 });
			for (unsigned int i = 0; i < bodies.getSize(); i++) {
				bodies[i]->tr1.pos *= scale;
			}
			td_vector<Shape*> shapes = *(td_vector<Shape*>*)FindDMAAddy(moduleBase + 0x438820, { 0x48, 0x158 });
			for (unsigned int i = 0; i < shapes.getSize(); i++) {
				shapes[i]->vox->scale *= scale;
				shapes[i]->local_tr.pos *= scale;
			}
			td_vector<Joint*> joints = *(td_vector<Joint*>*)FindDMAAddy(moduleBase + 0x438820, { 0x48, 0x198 });
			for (unsigned int i = 0; i < joints.getSize(); i++) {
				joints[i]->local_pos_parent *= scale;
				joints[i]->local_pos_child *= scale;
			}
			// TODO: wheels, water, boundary, etc.
		}
		if (GetAsyncKeyState(VK_F4) & 1) {
			const RGBA* snow_color = (RGBA*)FindDMAAddy(moduleBase + ~0x350DC0, { });
			RGBA new_color = OpenColorPicker(snow_color->r, snow_color->g, snow_color->b);
			Patch((BYTE*)snow_color, (BYTE*)&new_color, sizeof(RGBA));
		}
