#include "unpack.h"

bool PackDecompress(char* file)
{
    HANDLE hFile = CreateFileA(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(hFile, &file_size))
    {
        return false;
    }

    size_t in_size = file_size.QuadPart;
    BYTE* in_buffer = malloc(in_size);
    if (!in_buffer)
    {
        return false;
    }

    DWORD bytes_read = 0;
    if (!ReadFile(hFile, in_buffer, in_size, &bytes_read, NULL))
    {
        free(in_buffer);
        return false;
    }

    if (bytes_read != in_size)
    {
        free(in_buffer);
        return false;
    }

    DECOMPRESSOR_HANDLE hDecompressor = NULL;
    if (!CreateDecompressor(COMPRESS_ALGORITHM_XPRESS_HUFF, NULL, &hDecompressor))
    {
        free(in_buffer);
        return false;
    }


    size_t new_out_size = 0;
    Decompress(hDecompressor, in_buffer, file_size.QuadPart, NULL, 0, &new_out_size);

    size_t out_size = new_out_size;
    BYTE* out_buffer = malloc(out_size);

    if (!Decompress(hDecompressor, in_buffer, file_size.QuadPart, out_buffer, out_size, &new_out_size))
    {
        free(out_buffer);
        free(in_buffer);
        CloseCompressor(hDecompressor);
        return false;
    }

    out_buffer = realloc(out_buffer, new_out_size);
    free(in_buffer);
    CloseCompressor(hDecompressor);
    CloseHandle(hFile);

    hFile = CreateFileA(file, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD bytes_written = 0;
    if (!WriteFile(hFile, out_buffer, new_out_size, &bytes_written, NULL))
    {
        free(out_buffer);
        CloseHandle(hFile);
        return false;
    }

    if (bytes_written != new_out_size)
    {
        free(out_buffer);
        CloseHandle(hFile);
        return false;
    }

    free(out_buffer);
    CloseHandle(hFile);
    return true;
}

bool PackBuildStructure(char* file)
{
    HANDLE hFile = CreateFileA(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)  return false;


    BYTE byte = 0;
    DWORD bytes_read = 0;
    DWORD size = 0;

    DataFileState state = R_PATH;

    char   tmp_size[sizeof(size_t)];
    size_t file_size = 0;
    char* file_path = malloc(0);

    HANDLE hOutFile = INVALID_HANDLE_VALUE;


    char* outdir = malloc(strlen(file) + 5);
    if (!outdir) return false;

    outdir = strcpy(outdir, file);
    outdir = strcat(outdir, "_out");

    if (!PathIsDirectoryA(outdir))
    {
        if (!CreateDirectoryA(outdir, NULL))
        {
            free(outdir);
            return false;
        }
    }

    while (ReadFile(hFile, &byte, sizeof(byte), &bytes_read, NULL) && bytes_read > 0)
    {
        switch (state)
        {
        case R_PATH:
        {
            file_path = realloc(file_path, size + 1);
            file_path[size] = byte;
            size++;

            if (byte != 0)
            {
                break;
            }

            
            char* file_dest = PackGetOutName(outdir, file_path);
            if (file_dest == NULL)
            {
                free(file_path);
                free(outdir);
                return false;
            }

            char* dir_dest = malloc(strlen(file_dest) + 1);
            if (dir_dest == NULL)
            {
                free(file_path);
                free(outdir);

                free(file_dest);
                return false;
            }

            strcpy(dir_dest, file_dest);
            PathRemoveFileSpecA(dir_dest);
            
            if (!PathIsDirectoryA(dir_dest, NULL))
            {
                if (!CreateDirectoryA(dir_dest, NULL))
                {
                    free(file_path);
                    free(outdir);

                    free(file_dest);
                    free(dir_dest);

                    return false;
                }
            }
            
            hOutFile = CreateFileA(file_dest, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hOutFile == INVALID_HANDLE_VALUE)
            {                
                free(file_path);
                free(outdir);

                free(file_dest);
                free(dir_dest);

                return false;
            }

            free(file_dest);
            free(dir_dest);

            file_path = realloc(file_path, 0);

            state = R_FILESIZE;
            size = 0;
        }
        break;
        case R_FILESIZE:
        {
            tmp_size[size] = byte;
            size += 1;

            if (size >= sizeof(size_t))
            {
                memmove(&file_size, tmp_size, sizeof(size_t));

                state = R_FILEDATA;
                size = 0;
            }
        }
        break;
        case R_FILEDATA:
        {
            WriteFile(hOutFile, &byte, sizeof(BYTE), 0, 0);
            size = file_size - 1;

            char buffer[BUFFER_SIZE];
            size_t next_size = BUFFER_SIZE;
            while(1)
            {
                if ((int)(size - next_size) <= 0)
                {
                    next_size = size;
                }

                if (ReadFile(hFile, &buffer, next_size, &bytes_read, NULL) && bytes_read > 0)
                {
                    WriteFile(hOutFile, &buffer, next_size, 0, NULL);

                    size -= next_size;
                    if (size <= 0)
                    {
                        break;
                    }
                }
                else {
                    break;
                }
            }

            CloseHandle(hOutFile);
            hOutFile = INVALID_HANDLE_VALUE;

            state = R_PATH;
            file_size = 0;
            size = 0;
        }
        break;
        }
    }

    CloseHandle(hFile);
    free(file_path);
    free(outdir);
    return true;
}

char* PackGetOutName(char* base_path, char* ext_path) 
{
    size_t base_len = strlen(base_path);
    size_t ext_len = strlen(ext_path);

    size_t findex = 0;
    for (size_t i = 0; i < ext_len; i++)
    {
        if (ext_path[i] == '\\')
        {
            findex = i;
            ext_path[i] = '_';
        }
        else if (ext_path[i] == ':' || ext_path[i] == '.')
        {
            ext_path[i] = '_';
        }
    }

    ext_path[findex] = '\\';


    char* outpath = malloc(base_len + ext_len + 2);
    memcpy(outpath, base_path, base_len + 1);
    outpath[strlen(outpath)] = '\\';
    memcpy(outpath + base_len + 1, ext_path, ext_len + 1);
    return outpath;
}