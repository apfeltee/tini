
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tini.h"


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
        if((c == '\r') && (str[(*iter) + 1] == '\n'))
        {
             *iter += 1;
            //continue;
        }
        else
        {
            if(c == '\n')
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

static bool casecmp_s(const char* stra, size_t alen, const char* strb, size_t blen)
{
    int ca;
    int cb;
    size_t i;
    if(alen != blen)
    {
        return false;
    }
    for(i=0; i<alen; i++)
    {
        ca = stra[i];
        cb = strb[i];
        if(toupper(ca) != toupper(cb))
        {
            return false;
        }
    }
    return true;
}

#if 0
static bool casecmp(const char* stra, const char* strb)
{
    return casecmp_s(stra, strlen(stra), strb, strlen(strb));
}
#endif

static void nul_section(tiniparser_t* t)
{
    memset(t->section, 0, TINI_MAX_SECTION);
    t->slength = 0;
}

static void nul_key(tiniparser_t* t)
{
    memset(t->key, 0, TINI_MAX_KEY);
    t->klength = 0;
}

static void nul_value(tiniparser_t* t)
{
    memset(t->value, 0, TINI_MAX_VALUE);
    t->vlength = 0;
}

static void nul_all(tiniparser_t* t)
{
    nul_section(t);
    nul_key(t);
    nul_value(t);
}

void tiniparser_default_section(tiniparser_t* t, const char* str)
{
    strcpy(t->section, str);
    t->slength = strlen(str);
}

void tiniparser_init(tiniparser_t* t)
{
    nul_all(t);
    tiniparser_default_section(t, "main");
    t->iter = 0;
    t->lineno = 0;
}

bool tiniparser_each(tiniparser_t* t, const char* str, size_t slen)
{
    int pos;
    size_t llen;
    char line[TINI_MAX_LINE + 1];
    while(true)
    {
        memset(line, 0, TINI_MAX_LINE);
        /*
        * eachline() will only ever read as much as TINI_MAX_LINE ...
        * so if the line is larger than TINI_MAX_LINE, then
        * a) are you sure you understood what ini files are used for?
        * b) it won't affect parsing (at least until i implement quotes)
        */
        if(eachline(line, TINI_MAX_LINE, &llen, &t->iter, &t->lineno, str, slen))
        {
            llen = trimspaces(line, line, llen);
            if((line[0] == '#') || (line[0] == ';'))
            {
                /* comment. move along */
                continue;
            }
            else if((line[0] == '[') && (line[llen - 1] == ']'))
            {
                if((llen - 2) > TINI_MAX_SECTION)
                {
                    fprintf(stderr, "error: section string too long");
                    return false;
                }
                memset(t->section, 0, TINI_MAX_SECTION);
                memcpy(t->section, line + 1, llen - 2);
                t->slength = llen - 2;
            }
            else
            {
                if((pos = spos(line, llen, '=')) != -1)
                {
                    if(pos > TINI_MAX_KEY)
                    {
                        fprintf(stderr, "error: key string too long\n");
                        return false;
                    }
                    nul_key(t);
                    memcpy(t->key, line, pos);
                    t->klength = trimspaces(t->key, t->key, pos);
                    if(((llen - pos) - 1) > TINI_MAX_VALUE)
                    {
                        fprintf(stderr, "error: value string too long");
                        return false;
                    }
                    nul_value(t);
                    memcpy(t->value, line + (pos + 1), (llen - pos) - 1);
                    t->vlength = trimspaces(t->value, t->value, (llen - pos) + 1);
                    return true;
                }
                else
                {
                    fprintf(stderr, "error: bad key<->value line near line %zu\n", t->lineno);
                    return false;
                }
            }
        }
        else
        {
            break;
        }
    }
    return false;
}

bool tiniparser_is_section(tiniparser_t* t, const char* str)
{
    if(t->slength > 0)
    {
        return casecmp_s(t->section, t->slength, str, strlen(str));
    }
    return false;
}

bool tiniparser_is_key(tiniparser_t* t, const char* str)
{
    if(t->klength > 0)
    {
        return casecmp_s(t->key, t->klength, str, strlen(str));
    }
    return false;
}
