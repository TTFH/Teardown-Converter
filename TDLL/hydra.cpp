#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

// g++ -Wall -Wextra -Werror -Wpedantic -s -shared -static hydra.cpp -o hydra5-x64.dll

#define EXPORT(exp_name, target_name) asm(".section .drectve\n\t.ascii \" -export:" #exp_name "=" #target_name "\"")

EXPORT(hydra5_init, hydra5.hydra5_init);
EXPORT(hydra5_term, hydra5.hydra5_term);
EXPORT(hydra5_update, hydra5.hydra5_update);
EXPORT(hydra5_client_create, hydra5.hydra5_client_create);
EXPORT(hydra5_get_error_message, hydra5.hydra5_get_error_message);
EXPORT(hydra5_client_connect_steam, hydra5.hydra5_client_connect_steam);
EXPORT(hydra5_disconnect_everything, hydra5.hydra5_disconnect_everything);
EXPORT(hydra5_is_executing_requests, hydra5.hydra5_is_executing_requests);
//EXPORT(hydra5_telemetry_typed_event, hydra5.hydra5_telemetry_typed_event);
EXPORT(hydra5_client_connect_developer, hydra5.hydra5_client_connect_developer);
EXPORT(hydra5_client_get_kernel_session_id_visual_alias, hydra5.hydra5_client_get_kernel_session_id_visual_alias);

/*
DLCActivatedBiEvent
LevelEndBiEvent
LevelStartBiEvent
ModManagerBiEvent
SettingsBiEvent
ToolUpgradeBiEvent
TutorialBiEvent
*/
extern "C" __declspec(dllexport) void hydra5_telemetry_typed_event(const char* event, int one, void* param) {
	printf("[Hydra] %s %d 0x%p\n", event, one, param);
}

DWORD WINAPI MainThread(HMODULE hModule) {
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);
    printf("Hello World!\n");
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	(void)lpvReserved;
	if (fdwReason == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
	return TRUE;
}
