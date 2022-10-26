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

typedef BOOL(__stdcall* wglSwapBuffers_t) (HDC hDc);
wglSwapBuffers_t wglSwapBuffers;

Hook SwapBuffersHook("wglSwapBuffers", "opengl32.dll", (BYTE*)hkwglSwapBuffers, (BYTE*)&wglSwapBuffersGateway, 5);
if (GetAsyncKeyState(VK_F5) & 1)
	SwapBuffersHook.Enable();
