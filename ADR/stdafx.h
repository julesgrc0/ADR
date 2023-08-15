#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

// #define UNICODE

#include <iostream>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <mutex>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <windows.h>
#include <Lmcons.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <dpapi.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <RestartManager.h>
#include <KnownFolders.h>

#undef min
#undef max

// #pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "rstrtmgr.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#include <curl/curl.h>

#include "json/json.hpp"
#include "zip/zip.h"

using json = nlohmann::json;