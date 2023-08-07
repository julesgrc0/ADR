#pragma once
#include <Windows.h>

#define HIDE_FUNCTION(modName, funcName, funcRet, funcSign, funcErr)            \
hModule = GetModuleHandleA(modName);                                    \
if (hModule == NULL) hModule = LoadLibraryA(modName);                           \
if (hModule == NULL) return funcErr;                                            \
funcRet(*hide_##funcName)funcSign = (funcRet(*)funcSign)GetProcAddress(hModule, #funcName);