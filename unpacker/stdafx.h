#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC 
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <memory.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>
#include <compressapi.h>
#include <shlwapi.h>

#pragma comment(lib, "cabinet.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG || _DEBUG_EXE
#include <crtdbg.h>
#endif

#define ADR_LOG(x, ...) printf("[ADR] " x "\n", __VA_ARGS__)
#define ADR_PORT 12345
#define BUFFER_SIZE 4096 //BUFSIZ * 8
