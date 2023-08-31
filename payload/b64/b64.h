#pragma once
#include "../stdafx.h"

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char*		Base64Encode(const char* data, size_t input_length);
char*		Base64Decode(const char* data, size_t input_length, size_t* output_length);