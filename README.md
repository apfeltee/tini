## tini

A tiny ini file reader, written in plain ole ANSI C, using minimal memory.

Usage is extremely straight-forward:

```C
....
#include "tini.h"
....
tiniparser_t t;
tiniparser_init(&t);
while(tiniparser_each(&t, inputstring, inputlen))
{
    /**
    * t->section contains the current section (defaults to "main"), t->slength its length,
    * t->key contains the current key, and t->klength its length,
    * t->value contains the current value, and t->vlength its length.
    *
    * note that you should **copy** the values - the pointers are reset with each loop.
    */
    printf("in '%s': \"%.*s\"=\"%.*s\"\n", t->section, (int)(t->klength), t->key, (int)(t->vlength), t->value);
}
....
```