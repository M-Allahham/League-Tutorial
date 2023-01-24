#include <conio.h>
#include <stdio.h>
#include "Ntfuncs.h"
#include <iostream>

#define pl(s) printf("%s\n", s)

HANDLE GetProcHandle(const wchar_t* szProcName)
{
	HANDLE hProc = NULL;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnap, &pe32))
	{
		if (!wcscmp(szProcName, pe32.szExeFile))
		{
			hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		}
		else
		{
			while (Process32Next(hSnap, &pe32))
			{
				if (!wcscmp(szProcName, pe32.szExeFile))
				{
					hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
					break;
				}
			}
		}
	}

	return hProc;
}

int main()
{
	HANDLE hProc = GetProcHandle(L"League of Legends.exe");

	if (hProc)
	{
		pl("Found Process!");
	}
	else
	{
		pl("Run target then run me again.");
	}

	LPCSTR DllPath = "";

	LPVOID pDllPath = FVirtualAllocEx(hProc, 0, strlen(DllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

	WriteProcessMemory(hProc, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, 0);

	HANDLE hLoadThread = CreateRemoteThread(hProc, 0, 0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"),
			"LoadLibraryA"), pDllPath, 0, 0);

	WaitForSingleObject(hLoadThread, INFINITE);

	char test;

	std::cout << "Dll path allocated at: " << std::hex << pDllPath << std::endl;
	std::cin >> test;

	if (hProc)
		CloseHandle(hProc);
	return 0;
}