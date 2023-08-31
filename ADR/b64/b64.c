#include "b64.h"

char* Base64Encode(const char* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = (char*)malloc(output_length + 1);
    if (!encoded_data) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        unsigned __int32 octet_a = i < input_length ? data[i++] : 0;
        unsigned __int32 octet_b = i < input_length ? data[i++] : 0;
        unsigned __int32 octet_c = i < input_length ? data[i++] : 0;

        unsigned __int32 triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }

    while (j > output_length) {
        encoded_data[--j] = '=';
    }

    encoded_data[output_length] = '\0';
    return encoded_data;
}

char* Base64Decode(const char* data, size_t input_length, size_t* output_length) {
    if (input_length % 4 != 0) return NULL;

    *output_length = (input_length / 4) * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    char* decoded_data = (char*)malloc(*output_length);
    if (!decoded_data) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        unsigned __int32 sextet_a = data[i] == '=' ? 0 & i++ : base64_chars[(size_t)data[i++]];
        unsigned __int32 sextet_b = data[i] == '=' ? 0 & i++ : base64_chars[(size_t)data[i++]];
        unsigned __int32 sextet_c = data[i] == '=' ? 0 & i++ : base64_chars[(size_t)data[i++]];
        unsigned __int32 sextet_d = data[i] == '=' ? 0 & i++ : base64_chars[(size_t)data[i++]];

        unsigned __int32 triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;
    }

    return decoded_data;
}