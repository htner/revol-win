#pragma once
#include <WinNT.h>
namespace PELoader
{

	ULONG GetProcAddress(ULONG uModuleBase, const char *name)
	{

		PIMAGE_DOS_HEADER  pDosHeader = (PIMAGE_DOS_HEADER)uModuleBase;
		if ( pDosHeader->e_magic != IMAGE_DOS_SIGNATURE )
			return 0;

		PIMAGE_NT_HEADERS  pNtHeader =(PIMAGE_NT_HEADERS)( (ULONG)uModuleBase + pDosHeader->e_lfanew );
		PIMAGE_DATA_DIRECTORY directory = &(pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);

		if ( !directory || !directory->Size)
			return 0;

		PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)(uModuleBase + directory->VirtualAddress);
		if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0)
			return 0;

		// search function name in list of exported names
		int idx = -1;
		DWORD* nameRef = (DWORD *)(uModuleBase + exports->AddressOfNames);
		WORD* ordinal = (WORD *)(uModuleBase + exports->AddressOfNameOrdinals);
		for (DWORD i=0; i<exports->NumberOfNames; i++, nameRef++, ordinal++)
		{
			if (stricmp(name, (const char *)(uModuleBase + *nameRef)) == 0)
			{
				idx = *ordinal;
				break;
			}
		}

		if (idx == -1)
			return NULL;

		if ((DWORD)idx > exports->NumberOfFunctions)
			// name <-> ordinal number don't match
			return NULL;

		// AddressOfFunctions contains the RVAs to the "real" functions
		return (ULONG)(uModuleBase + *(DWORD *)(uModuleBase + exports->AddressOfFunctions + (idx*4)));
	}
}