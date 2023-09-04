#pragma once
#include "stdafx.h"
#include "b64/b64.h"

#ifdef _DEBUG || _DEBUG_EXE
#define DBG_FILE_SIZE(path)\
{\
WIN32_FILE_ATTRIBUTE_DATA fileInfo;\
if (GetFileAttributesExA(path, GetFileExInfoStandard, &fileInfo))\
{\
    ULARGE_INTEGER fileSize;\
    fileSize.LowPart = fileInfo.nFileSizeLow;\
    fileSize.HighPart = fileInfo.nFileSizeHigh;\
    printf("%s: (%llu)\n", path, fileSize.QuadPart);\
}\
}
#endif // _DEBUG || _DEBUG_EXE


wchar_t*	StringToWString(const char* str);
char*		WStringToString(const wchar_t* wstr);

char*		GetEncryptionKeyString(const char* path);
char*		ExtractEncyrptionKey(const char* key);

bool		TerminateBusyFileProc(const char* path);
bool		IsFileBusy(const char* path);

char*		GetWinFolderById(REFKNOWNFOLDERID id);
bool		IsValidDirectory(const WIN32_FIND_DATAA findData);