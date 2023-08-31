#pragma once
#include "stdafx.h"
#include "b64/b64.h"

wchar_t*	StringToWString(const char* str);
char*		WStringToString(const wchar_t* wstr);


/*
 FILE_DISPOSITION_INFO fdi;
            fdi.DeleteFile = TRUE;      //  Marking for deletion
            Success = SetFileInformationByHandle(
                CompressedFile,
                FileDispositionInfo,
                &fdi,
                sizeof(FILE_DISPOSITION_INFO));
*/

char* GetEncryptionKeyString(const char* path);
char* ExtractEncyrptionKey(const char* key);

bool TerminateBusyFileProc(const char* path);
bool IsFileBusy(const char* path);

char*	GetWinFolderById(REFKNOWNFOLDERID id);
bool	IsValidDirectory(const WIN32_FIND_DATAA findData);