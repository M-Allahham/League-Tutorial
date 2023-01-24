#pragma once

#include "Ntfuncs.h"
#include "Syscall.h"
#include <sysinfoapi.h>
#include <VersionHelpers.h>

LPCSTR RtlInterlockedCompareExchange64 = "RtlInterlockedCompareExchange64"; // szRtlInterlockedCompareExchange64;

LPVOID FVirtualAllocEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD flProtect)
{

	int loc = 0x170;

	OSVERSIONINFOEX info;
	ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	GetVersionEx((LPOSVERSIONINFO)&info);

	if (info.dwMajorVersion >= 10)
	{
		if (info.dwMinorVersion >= 22000)
			loc = 0x180;
		else
			loc = 0x170;
	}

	void* v10 = lpAddress;
	SIZE_T v11 = dwSize;
	DWORD nndPreferred = 0xFFFFFFFF;
	DWORD v6 = flAllocationType & 0xFFFFFFC0;
	if (nndPreferred != -1)
		v6 |= nndPreferred + 1;
	DWORD v7 = flProtect;
	DWORD v8 = v6;

	NTSTATUS ntStatus = Syscall<NTSTATUS>({ 0x18 }, RtlInterlockedCompareExchange64, 0x180, { 0x18 })
		(hProcess, &v10, 0, &v11, v8, v7);
	if ((signed int)ntStatus >= 0)
		return v10;
	return 0;
}