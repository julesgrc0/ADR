#include "stdafx.h"

#include "sock/sock.h"
#include "unpack/unpack.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ char* pCmdLine, _In_ int nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

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

	if (__argc <= 1)
	{
        StartServer();
	}
	else
	{
        ADR_LOG("Unpacker Mode");
        for (size_t i = 1; i < __argc; i++)
        {
            char* file = __argv[i];
            if (!PathFileExistsA(file))
            {
                ADR_LOG("Pack not found (%s)", file);
                continue;
            }

            if (!PackDecompress(file))
            {
                ADR_LOG("Pack decompression failed (%s)", file);
                continue;
            }

            if (!PackBuildStructure(file))
            {
                ADR_LOG("Pack structure corrupted (%s)", file);
                continue;
            }

            ADR_LOG("Unpack success (%s)", file);
        }
	}

    
#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif // _DEBUG

    WaitForInputIdle(GetCurrentProcess(), INFINITE);
    return EXIT_SUCCESS;
}