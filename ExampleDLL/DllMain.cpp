#include <Windows.h>
#include <psapi.h>

// The entry point of the DLL, read up here: https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		wchar_t pathName[MAX_PATH];
		// Get the current process' filepath and put it in the wchar_t array
		GetProcessImageFileNameW(GetCurrentProcess(), pathName, sizeof(pathName) / sizeof(*pathName));
		// Output the process path into a cute lil message box :)
		MessageBoxW(NULL, pathName, L"Process path", MB_ICONINFORMATION);
		break;
	}

	// The DLL successfully attached to the target process (hinstDLL)
	// This means the DLL is located in the target process' address space and can utilize its code!
	return TRUE;
}