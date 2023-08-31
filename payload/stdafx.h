#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <memory.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>

#include <knownfolders.h>
#include <shlwapi.h>
#include <compressapi.h>
#include <restartmanager.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "cabinet.lib")
#pragma comment(lib, "rstrtmgr.lib")

#ifdef _DEBUG_EXE
#include <crtdbg.h>
#endif // _DEBUG_EXE

#define BUFFER_SIZE 4096 // BUFSIZ * 8


/*
#include <lmcons.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <dpapi.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

*/


#define SAFE_FREE(x)\
if(x != NULL)\
{\
	free(x);\
}

#define NULL_RET(x, ret)\
if(x == NULL)\
{\
return ret;\
}
