#include <cstdio>
#include <windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <iostream>

// This function checks all processes on the system to see if it's League
// If so, get HANDLE to process and return it
HANDLE GetProcessHandle() {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    // Make snapshot of all processes in the system
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    // Check that first process exists
    if (Process32First(snapshot, &entry) == TRUE) {
        // Iterate through all processes
        while (Process32Next(snapshot, &entry) == TRUE) {
            // If process' executable file name is League of Legends.exe...
            if (strcmp(entry.szExeFile, "League of Legends.exe") == 0) {
                // Return the process' HANDLE
                CloseHandle(snapshot);
                return OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
            }
        }
    }
}

// Given a handle to a process, return it's base address
DWORD GetBaseAddress(HANDLE hProcess) {
    HMODULE lphModule[1024];
    DWORD lpcbNeeded;
    // Get all of the hProcess's module handles into hMods
    if (EnumProcessModules(hProcess, lphModule, sizeof(lphModule), &lpcbNeeded)) {
        // Return the address of the first module (base address)
        return (DWORD_PTR)lphModule[0];
    }
    return 0;
}

int main(int, char* [])
{
    // Initializing League's handle and base address
    HANDLE hProcess = 0;
    DWORD moduleBaseAddr = 0;

    // Execute functions above
    hProcess = GetProcessHandle();
    moduleBaseAddr = GetBaseAddress(hProcess);
    
    // Game loop...
    while (true) {
        // Variables
        SIZE_T numBytes;
        DWORD playerAddr = 0;
        float Health = 0;
        float MaxHealth = 0;

        // Get player's address
        ReadProcessMemory(hProcess, (void*)(moduleBaseAddr + 0x3163080), &playerAddr, sizeof(DWORD), &numBytes);

        // Get health and max health of player
        ReadProcessMemory(hProcess, (LPCVOID)(playerAddr + 0xE7C), &Health, sizeof(float), &numBytes);
        ReadProcessMemory(hProcess, (LPCVOID)(playerAddr + 0xE8C), &MaxHealth, sizeof(float), &numBytes);

        // When we drop below 25% HP...
        if (Health / MaxHealth < 0.25) {
            // Press the "F" key down...
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = 33;
            //input.ki.time = 0;
            //input.ki.dwExtraInfo = 0;
            //input.ki.wVk = 0;
            input.ki.dwFlags = KEYEVENTF_SCANCODE;
            SendInput(1, &input, sizeof(INPUT));
            Sleep(5);

            // Release the "F" key
            input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));

            std::cout << "Healed at " << Health / MaxHealth << " health" << std::endl;
            Sleep(200);
        }
    }
    CloseHandle(hProcess);

    return 0;
}