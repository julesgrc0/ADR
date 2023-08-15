#define _CRT_SECURE_NO_WARNINGS
#include "adr.h"
#include "stdafx.h"

#ifdef _DEBUG_EXE
#define DLL_EXPORT
#else
#define DLL_EXPORT extern "C" __declspec(dllexport)
#endif

DLL_EXPORT int CALLBACK main() {
  ADR* adr = new ADR();
  return adr->start();
}

#ifdef _DEBUG_EXE
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ char* pCmdLine, _In_ int nCmdShow) {
  if (AllocConsole()) {
    FILE* fDummy;

    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);

    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();
  }
  main();
  return 0;
}
#else
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}
#endif  // _DEBUG_EXE
