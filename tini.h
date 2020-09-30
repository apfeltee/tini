
#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct tini_tag_t tini_t;
typedef bool(*tini_callback_t)(tini_t*, void*);
enum
{
    TINI_MAX_SECTION = 512,
    TINI_MAX_KEY = 512,
    TINI_MAX_VALUE = (1024 * 2),
    TINI_MAX_LINE = (TINI_MAX_KEY + TINI_MAX_VALUE) + 1

};

struct tini_tag_t
{
    char section[TINI_MAX_SECTION + 1];
    size_t slength;

    char key[TINI_MAX_KEY + 1];
    size_t klength;

    char value[TINI_MAX_VALUE + 1];
    size_t vlength;
};

extern bool tini_parse_string(const char* str, size_t slen, tini_callback_t cb, void* up);
extern bool tini_parse_file(const char* filename, tini_callback_t cb, void* up);

