#pragma once

#include "stdafx.h"

#define SEPARATOR ';'
#define START_SEPARATOR '<'
#define END_SEPARATOR '>'
#define DEFAULT_BUFFER_SIZE 1024

inline void ReadDirectory(const std::string& path, std::function<bool(const WIN32_FIND_DATAA&)> handleFunc)
{
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA((path + "\\*").c_str(), &findData);

    if (findHandle == INVALID_HANDLE_VALUE) {
        FindClose(findHandle);
        return;
    }

    do {
        if (!handleFunc(findData))
        {
            break;
        }
    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);
}

inline bool IsValidDirectory(const WIN32_FIND_DATAA& findData)
{
    return (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0;
}