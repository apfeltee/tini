
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tini.h"

static void nul_section(tini_t* t)
{
    memset(t->section, 0, TINI_MAX_SECTION);
    t->slength = 0;
}

static void nul_key(tini_t* t)
{
    memset(t->key, 0, TINI_MAX_KEY);
    t->klength = 0;
}

static void nul_value(tini_t* t)
{
    memset(t->value, 0, TINI_MAX_VALUE);
    t->vlength = 0;
}

static void nul_all(tini_t* t)
{
    nul_section(t);
    nul_key(t);
    nul_value(t);
}

static void set_default(tini_t* t)
{
    strcpy(t->section, "main");
    t->slength = 4;
}

/*
* read a whole file into memory. don't use with very large files,
* as it obviously will be stored in RAM; lest you have RAM to spare,
* in which case, knock yourself out.
* this was born out of lazyness, and it is, in fact, not
* a good way to read a whole (big) file (which *should* be read chunk-wise).
* tl;dr use it for small files only.
*
* $buf: where the data will be stored. will be malloc()'d, so also
*       needs to be free()'d.
* $fh: file handle.
*
* returns the amount of bytes read.
*/
static size_t readfile(char** buf, FILE* fh)
{
    size_t sz;
    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    fseek(fh, 0, SEEK_SET);
    *buf = (char*)malloc(sz + 1);
    /* should check rt val... */
    fread(*buf, sizeof(char), sz, fh);
    fclose(fh);
    (*buf)[sz] = 0;
    return sz;
}

/*
* trim spaces front and back of a string
*
* $out: where the "new" string is stored - it must be at least a big as $len
* $str: the input string
* $len: the size of $str
*
* can be used with out == str, i.e.:
*
*   ...
*   inputlen = trimspaces(inputstring, inputstring, inputlen);
*   ...
*
* which would consequently update inputstring and inputlen to the
* now trimmed string.
*/
static size_t trimspaces(char *out, const char* str, size_t len)
{
    size_t nlen;
    const char *end;
    nlen = len;
    if(len == 0)
    {
        return 0;
    }
    /* trim leading space */
    while(isspace((int)(str[0])))
    {
        str++;
        nlen--;
    }
    /* all spaces ... */
    if(*str == 0)
    {
        *out = 0;
        return 1;
    }
    /* trim trailing space */
    end = str + (nlen - 1);
    while(end > str && isspace((unsigned char)*end))
    {
        end--;
        nlen--;
    }
    end++;
    /* copy trimmed string and add null terminator */
    memcpy(out, str, nlen);
    out[nlen] = 0;
    return nlen;
}

/*
* iterate a string line-wise
*
* $dest: where the line is finally stored
* $mlen: maximum size of $dest
* $llen: where the line length is stored
* $iter: the iterator used internally - *must* be set to 0 prior to call
* $lco: where the current line number is stored - *must* be se to 0 prior to call
* $str: input string
* $slen: size of input string
*
* example usage:
*
*  size_t lco;
*  size_t iter;
*  size_t llen;
*  char line[1024 + 1];
*  lco = iter = llen = 0;
*  while(eachline(line, 1024, &llen, &iter, &lco, inputstring, inputlength))
*  {
*      ...
*  }
*
*/
static bool eachline(char* dest, size_t mlen, size_t* llen, size_t* iter, size_t* lco, const char* str, size_t slen)
{
    int c;
    *llen = 0;
    memset(dest, 0, mlen);
    while(*iter < slen)
    {
        c = str[*iter];
        if(c == '\r')
        {
            *iter += 1;
        }
        else if(c == '\n')
        {
            *lco += 1;
            if(*llen > 0)
            {
                return true;
            }
        }
        else
        {
            dest[*llen] = c;
            *llen += 1;
        }
        *iter += 1;
    }
    return false;
}

/*
* search and return position of $findme in $str.
* returns -1 if not found.
*/
static int spos(const char* str, size_t slen, int findme)
{
    int p;
    for(p=0; p<(int)slen; p++)
    {
        if(str[p] == findme)
        {
            return p;
        }
    }
    return -1;
}

bool tini_parse_string(const char* str, size_t slen, tini_callback_t cb, void* up)
{
    int pos;
    size_t iter;
    size_t llen;
    size_t lco;
    tini_t t;
    char line[TINI_MAX_LINE + 1];
    iter = 0;
    lco = 0;
    nul_all(&t);
    set_default(&t);
    while(true)
    {
        if(eachline(line, TINI_MAX_LINE, &llen, &iter, &lco, str, slen))
        {
            llen = trimspaces(line, line, llen);
            if((line[0] == '[') && (line[llen - 1] == ']'))
            {
                memcpy(t.section, line + 1, llen - 2);
                t.slength = llen - 2;
                #if 0
                fprintf(stderr, "t.section=%.*s\n", t.slength, t.section);
                #endif
            }
            else
            {
                if((pos = spos(line, llen, '=')) != -1)
                {
                    nul_key(&t);
                    nul_value(&t);
                    memcpy(t.key, line, pos);
                    t.klength = trimspaces(t.key, t.key, pos);
                    memcpy(t.value, line + (pos + 1), (llen - pos) - 1);
                    t.vlength = trimspaces(t.value, t.value, (llen - pos) + 1);
                    #if 0
                    fprintf(stderr, "in \"%.*s\": key=\"%.*s\" value=\"%.*s\"\n",
                        t.slength, t.section,
                        t.klength, t.key,
                        t.vlength, t.value
                    );
                    #endif
                    if(cb(&t, up) == false)
                    {
                        break;
                    }
                }
                else
                {
                    fprintf(stderr, "error: bad key<->value line near line %ld\n", lco);
                    return false;
                }
            }
        }
        else
        {
            break;
        }
    }
    return true;
}

bool tini_parse_file(const char* filename, tini_callback_t cb, void* up)
{
    bool rt;
    size_t sz;
    char* buf;
    FILE* fh;
    if((fh = fopen(filename, "rb")) == NULL)
    {
        return false;
    }
    sz = readfile(&buf, fh);
    fclose(fh);
    rt = tini_parse_string(buf, sz, cb, up);
    free(buf);
    return rt;
}

