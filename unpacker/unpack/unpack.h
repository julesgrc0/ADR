#pragma once
#include "../stdafx.h"

typedef enum DataFileState {
    R_PATH = 0,
    R_FILESIZE,
    R_FILEDATA
}DataFileState;

bool PackDecompress(char* file);

bool PackBuildStructure(char* file);

char* PackGetOutName(char* base, char* ext_path);