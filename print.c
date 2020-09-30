
#include <stdio.h>
#include "tini.h"

static bool prnvals(tini_t* t, void* up)
{
    (void)up;
    printf("in '%s': \"%.*s\"=\"%.*s\"\n", t->section, (int)(t->klength), t->key, (int)(t->vlength), t->value);
    return true;
}

int main(int argc, char* argv[])
{
    int i;
    const char* file;
    if(argc > 1)
    {
        for(i=1; i<argc; i++)
        {
            file = argv[i];
            if(!tini_parse_file(file, prnvals, NULL))
            {
                fprintf(stderr, "failed to process file '%s'\n", file);
            }
        }
    }
    return 0;
}

