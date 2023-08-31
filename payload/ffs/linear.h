#pragma once
#include "../stdafx.h"
#include "../utils.h"

typedef struct FFSearch
{
	char** elements;
	size_t len;
} FFSearch;


FFSearch*	FFCreateResult();
void		FFSearchLinearDir(const char* path, const char* search, FFSearch* out);
void		FFDestroyResult(FFSearch*);