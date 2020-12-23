
#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct tiniparser_tag_t tiniparser_t;
enum
{
    #if !defined(TINI_MAX_SECTION)
    TINI_MAX_SECTION = 512,
    #endif

    #if !defined(TINI_MAX_KEY)
    TINI_MAX_KEY = 512,
    #endif

    #if !defined(TINI_MAX_VALUE)
    TINI_MAX_VALUE = (1024 * 2),
    #endif

    /*
    * how big a total line may be.
    * *must* be big enough to hold TINI_MAX_KEY + TINI_MAX_VALUE + 1 (the '=')
    */
    TINI_MAX_LINE = (TINI_MAX_KEY + TINI_MAX_VALUE) + 1
};

struct tiniparser_tag_t
{
    /*
    * the string value of the current section.
    * ie. [foo] -> "foo"
    * unless specified otherwise via tiniparser_default_section, defaults to "main"
    */
    char section[TINI_MAX_SECTION + 1];
    /* length of section string */
    size_t slength;

    /* the string value of the key in current key-value pair. */
    char key[TINI_MAX_KEY + 1];
    /* length of the key. */
    size_t klength;

    /* string value of the value in current key-value pair. */
    char value[TINI_MAX_VALUE + 1];
    /* length of the value. */
    size_t vlength;

    /* approximate current line number in file/string */
    size_t lineno;

    /* used internally. don't modify it! */
    size_t iter;
};


extern void tiniparser_init(tiniparser_t* t);
extern void tiniparser_default_section(tiniparser_t* t, const char* str);
extern bool tiniparser_each(tiniparser_t* t, const char* str, size_t slen);

/* return true if str is the current section. case-insensitive. */
extern bool tiniparser_is_section(tiniparser_t* t, const char* str);

/* return true if str is the current key in the kv pair. case-insensitive. */
extern bool tiniparser_is_key(tiniparser_t* t, const char* str);
