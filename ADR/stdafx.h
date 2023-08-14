#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

//#define UNICODE

#include <iostream>
#include <algorithm>
#include <functional>

#include <regex>
#include <mutex>

#include <string>
#include <random>
#include <vector>
#include <chrono>
#include <thread>

#include <sstream>
#include <fstream>


#include <ShlObj.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <KnownFolders.h>
#include <RestartManager.h>
#include <windows.h>
#include <winsock2.h>
#include <Lmcons.h>
#include <iphlpapi.h>
#include <dpapi.h>

#undef min
#undef max

//#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "rstrtmgr.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#include <curl/curl.h>
#include "json/json.hpp"
#include "zip/zip.h"

using json = nlohmann::json;