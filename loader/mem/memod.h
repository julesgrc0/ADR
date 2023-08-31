#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

typedef LPVOID HMEMORYMODULE;
typedef LPVOID HMEMORYRSRC;
typedef LPVOID HCUSTOMMODULE;


typedef LPVOID (*CustomAllocFunc)(LPVOID, SIZE_T, DWORD, DWORD, void*);
typedef BOOL (*CustomFreeFunc)(LPVOID, SIZE_T, DWORD, void*);
typedef HCUSTOMMODULE (*CustomLoadLibraryFunc)(LPCSTR, void*);
typedef FARPROC (*CustomGetProcAddressFunc)(HCUSTOMMODULE, LPCSTR, void*);
typedef void (*CustomFreeLibraryFunc)(HCUSTOMMODULE, void*);

HMEMORYMODULE MemoryLoadLibrary(const void*, size_t);
HMEMORYMODULE MemoryLoadLibraryEx(const void*, size_t, CustomAllocFunc,
                                  CustomFreeFunc, CustomLoadLibraryFunc,
                                  CustomGetProcAddressFunc,
                                  CustomFreeLibraryFunc, void*);

HMEMORYRSRC MemoryFindResource(HMEMORYMODULE, LPCTSTR, LPCTSTR);
HMEMORYRSRC MemoryFindResourceEx(HMEMORYMODULE, LPCTSTR, LPCTSTR, WORD);

FARPROC MemoryGetProcAddress(HMEMORYMODULE, LPCSTR);
DWORD MemorySizeofResource(HMEMORYMODULE, HMEMORYRSRC);
LPVOID MemoryLoadResource(HMEMORYMODULE, HMEMORYRSRC);

void MemoryFreeLibrary(HMEMORYMODULE);

int MemoryCallEntryPoint(HMEMORYMODULE);
int MemoryLoadString(HMEMORYMODULE, UINT, LPTSTR, int);
int MemoryLoadStringEx(HMEMORYMODULE, UINT, LPTSTR, int, WORD);

