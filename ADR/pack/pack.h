#pragma once
#include "../stdafx.h"
#include "../utils.h"

typedef struct Pack {
	char* path;
	HANDLE file;
}Pack;

#define PACK_HANDLE_CHECK(pack, access, mode, ret)\
if (pack->file == INVALID_HANDLE_VALUE)\
{\
	pack->file = CreateFileA(pack->path, access, mode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);\
	if (pack->file == INVALID_HANDLE_VALUE)  return ret;\
}

#define PACK_HANDLE_RESET(pack)\
if(pack->file != INVALID_HANDLE_VALUE)\
{\
	CloseHandle(pack->file);\
	pack->file = INVALID_HANDLE_VALUE;\
}



Pack*		PackCreate();
void		PackDestroy(Pack* pack);

bool		PackCompress(Pack** in);
bool		PackWriteFile(Pack* pack, const char* path);
bool		PackWriteBuffer(Pack* pack, const char* name, const char* buffer, size_t len);
bool		PackSend(Pack* pack, const char* host, unsigned short port);

void		PackTargetFiles(Pack* pack, char* base, char** targets, size_t len);
void		PackDirectoryFiles(Pack* pack, char* dirpath, char** extensions, size_t len);
