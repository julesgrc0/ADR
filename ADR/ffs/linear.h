#pragma once
#include "../stdafx.h"
#include "../utils.h"

static std::mutex m_mutex;

static bool IsValidDirectory(const WIN32_FIND_DATAA& findData)
{
    return (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0;
}

void PreloadSearchLinear(std::string path, const std::string& search,
    std::vector<std::string>& results,
    std::vector<std::string>& folders);
void SearchLinear(std::string path, const std::string& search,
    std::vector<std::string>& results);
void SearchParallelLinear(std::string target, const std::string& search,
    std::vector<std::string>& results);