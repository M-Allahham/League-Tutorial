#include "Ntdefs.h"

#include "lazy_importer.h"

FARPROC GetProcedureAddress(const HANDLE& hModule, LPCSTR pszProcName)
{
	IMAGE_DOS_HEADER* pdhDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(hModule);
	//Check if valid PE
	if (pdhDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;

	IMAGE_NT_HEADERS* pndNTHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pdhDosHeader->e_lfanew + (long)hModule);
	if (pndNTHeader->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	//Traverse the export table to see if we can find the export
	IMAGE_EXPORT_DIRECTORY* iedExports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(pndNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + reinterpret_cast<long>(hModule));
	long* pNames = reinterpret_cast<long*>(iedExports->AddressOfNames + reinterpret_cast<long>(hModule));
	short wOrdinalIndex = -1;
	for (DWORD i = 0; i < iedExports->NumberOfFunctions; i++)
	{
		char* pszFunctionName = reinterpret_cast<char*>(pNames[i] + reinterpret_cast<long>(hModule));

		if (lstrcmpA(pszFunctionName, pszProcName) == 0)
		{
			wOrdinalIndex = static_cast<short>(i);
			break;
		}
	}

	if (wOrdinalIndex == -1)
		return 0;

	//wIndex now holds the index of the function name in the names array, which is the index of the ordinal.
	//The ordinal also acts as the index of the address
	short* pOrdinals = reinterpret_cast<short*>(iedExports->AddressOfNameOrdinals + reinterpret_cast<long>(hModule));
	unsigned long* pAddresses = reinterpret_cast<unsigned long*>(iedExports->AddressOfFunctions + reinterpret_cast<long>(hModule));

	short wAddressIndex = pOrdinals[wOrdinalIndex];
	return reinterpret_cast<FARPROC>(pAddresses[wAddressIndex] + reinterpret_cast<long>(hModule));
}

void* GetModuleBase(LPCSTR moduleName)
{
#if defined( _WIN64 )
#define PEBOffset 0x60
#define LdrOffset 0x18
#define ListOffset 0x10
	unsigned long long pPeb = __readgsqword(PEBOffset);

	if (!moduleName)
		return *reinterpret_cast<void**>(pPeb + 0x10);

#elif defined( _WIN32 )
#define PEBOffset 0x30
#define LdrOffset 0x0C
#define ListOffset 0x0C
	unsigned long pPeb = __readfsdword(PEBOffset);

	if (!moduleName)
		return *reinterpret_cast<void**>(pPeb + 0x08);
#endif

	wchar_t wModuleName[100] = { 0 };
	LI_FN(MultiByteToWideChar)(CP_ACP, MB_PRECOMPOSED, moduleName, -1, wModuleName, 100);

	pPeb = *reinterpret_cast<decltype(pPeb)*>(pPeb + LdrOffset);
	PLDR_DATA_TABLE_ENTRY pModuleList = *reinterpret_cast<PLDR_DATA_TABLE_ENTRY*>(pPeb + ListOffset);
	while (pModuleList->DllBase)
	{
		if (!_wcsicmp(pModuleList->BaseDllName.Buffer, wModuleName))
			return pModuleList->DllBase;
		pModuleList = reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(pModuleList->InLoadOrderLinks.Flink);
	}
	return nullptr;
}

tNtAllocateVirtualMemory NtAllocateVirtualMemory = NULL;

LPVOID NtAllocateVirtualMemoryFunc(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD flProtect)
{
	if (!NtAllocateVirtualMemory)
	{
		auto skNtdll = "ntdll.dll";
		HMODULE hNtdll = reinterpret_cast<HMODULE>(GetModuleBase(std::string(skNtdll).c_str()));
		auto skNtAllocateVirtualMemory = "NtAllocateVirtualMemory";
		NtAllocateVirtualMemory = (tNtAllocateVirtualMemory)GetProcedureAddress(hNtdll, std::string(skNtAllocateVirtualMemory).c_str());
	}

	void* v10 = lpAddress;
	SIZE_T v11 = dwSize;
	DWORD nndPreferred = 0xFFFFFFFF;
	DWORD v6 = flAllocationType & 0xFFFFFFC0;
	if (nndPreferred != -1)
		v6 |= nndPreferred + 1;
	DWORD v7 = flProtect;
	DWORD v8 = v6;

	if ((signed int)NtAllocateVirtualMemory(hProcess, &v10, 0, &v11, v8, v7) >= 0)
		return v10;
	return 0;
}