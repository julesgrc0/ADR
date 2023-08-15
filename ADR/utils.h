#pragma once
#include "stdafx.h"

#ifdef _DEBUG
#define LOG(x) std::cout << x << '\n';
#else
#define LOG(x)
#endif  // _DEBUG

#define COLOR_0 0x0249dc
#define COLOR_1 0xefd27b
#define COLOR_2 0xEA1179
static const char BASE64_CHARS[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string HPathGetParent(std::string);
std::string HPathGetName(std::string path);
std::string HPathGetWinTmp(const std::string&);
std::string HPathGetWinFld(REFKNOWNFOLDERID);
bool HPathCanRead(const std::string&);
size_t HPathFileSize(const std::string&);

bool HProcTerminate(DWORD id);
bool HProcGetIdLockingFile(const std::string& path, std::vector<DWORD>& ids);
bool HProcTerminateTree(DWORD id);

std::vector<std::string> HPathGetFiles(const std::string&);
std::vector<std::string> HPathGetDrives();
void HPathReadDirectory(
    const std::string& path,
    std::function<bool(const WIN32_FIND_DATAA&)> handleFunc);

std::string HPathAdd(const std::string&, const std::string&, bool = false);

bool HPathIsFld(const std::string&);
bool HPathIsFile(const std::string&);
bool HPathExists(const std::string&);

wchar_t* HStringToANSI(const std::string&);
std::string HStringToUTF8(const std::wstring&);
std::string HStringGetUUID(char sep);
std::string HStringGetRandom(size_t len);

std::vector<std::string> HChunkFile(const std::string& path, size_t chunkSize);

std::vector<char> HBase64Decode(const std::string& in);
std::string HBase64Encode(const std::vector<char>& in);
std::string HGetDecryptKey(const std::string& filePath);

/*
BOOL EnableDebugPrivilege()
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;

    if(!OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES |
TOKEN_QUERY, &hToken ))
    {
        return FALSE;
    }

    if(!LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luid ))
    {
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if(!AdjustTokenPrivileges( hToken, false, &tkp, sizeof( tkp ), NULL, NULL ))
    {
        return FALSE;
    }

    if(!CloseHandle( hToken ))
    {
        return FALSE;
    }

    return TRUE;
}
*/