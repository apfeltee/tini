
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "tini.h"

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

static void prnvals(tiniparser_t* t)
{
    /* NB. don't use %zu, because msvcrt will choke on it - as in, crash. seriously. */
    printf("in '%s': (%d)\"%.*s\" = (%d)\"%.*s\"\n", t->section, t->klength, (int)(t->klength), t->key, t->vlength, (int)(t->vlength), t->value);
}

int main(int argc, char* argv[])
{
    int i;
    size_t sz;
    char* data;
    const char* file;
    FILE* fh;
    tiniparser_t t;
    tiniparser_init(&t);
    if(argc > 1)
    {
        for(i=1; i<argc; i++)
        {
            file = argv[i];
            
            if((fh = fopen(file, "rb")) == NULL)
            {
                fprintf(stderr, "failed to open file '%s'\n", file);
                return 1;
            }
            sz = readfile(&data, fh);
            while(tiniparser_each(&t, data, sz))
            {
                prnvals(&t);
            }
            free(data);
        }
    }
    return 0;
}

