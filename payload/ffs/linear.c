#include "linear.h"

FFSearch* FFCreateResult()
{
    FFSearch* ffs = malloc(sizeof(FFSearch));
    NULL_RET(ffs, NULL);

    ffs->elements = NULL;
    ffs->len = 0;
    return ffs;
}

void FFSearchLinearDir(const char* path, const char* search, FFSearch* out)
{
    char* search_path = (char*)malloc(strlen(path) + 3);
    NULL_RET(search_path, );

    strcpy(search_path, path);
    strcat(search_path, "\\*");


    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(search_path, &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        free(search_path);
        return;
    }

    do {
        if (!IsValidDirectory(findData)) continue;

        char* dirpath = malloc(MAX_PATH);
        if (dirpath == NULL) continue;

        if (!PathCombineA(dirpath, path, findData.cFileName))
        {
            free(dirpath);
            continue;
        }

        if (!strcmp(findData.cFileName, search))
        {
            
            out->elements = (char**)realloc(out->elements, (out->len + 1) * sizeof(char*));
            if (out->elements == NULL)
            {
                free(dirpath);
                break;
            }
            out->elements[out->len] = dirpath;
            out->len++;
        }
        else
        {
            FFSearchLinearDir(dirpath, search, out);
            free(dirpath);
        }

    } while (FindNextFileA(findHandle, &findData));

    free(search_path);
    FindClose(findHandle);
}

void FFDestroyResult(FFSearch* ffs)
{
    for (size_t i = 0; i < ffs->len; i++)
    {
        free(ffs->elements[i]);
    }

    free(ffs->elements);
    free(ffs);
}

