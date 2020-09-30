
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct tiniwriter_tag_t tiniwriter_t;
typedef bool(*tiniwriter_callback_t)(tiniwriter_t*, const char*, size_t, void*);

struct tiniwriter_tag_t
{
    tiniwriter_callback_t cb;
    int method;
    bool fileopened;
    FILE* recv_handle;
    const char* currentsection;
    
};

/* internal[[[ */
enum
{
    recv_undefined,
    recv_to_file,
    recv_to_string,
};
static bool wrap_to_file(tiniwriter_t* tw, const char* str, size_t len, void* up)
{
    (void)up;
    fwrite(str, sizeof(char), len, tw->recv_handle);
    return true;
}
/* ]]]lanretni */

void tiniwriter_init(tiniwriter_t* tw)
{
    tw->cb = NULL;
    tw->fileopened = false;
    tw->method = recv_undefined;
    tw->recv_handle = NULL;
}

void tiniwriter_fini(tiniwriter_t* tw)
{
    if(tw->method == recv_to_file)
    {
        if(tw->fileopened)
        {
            fclose(tw->recv_handle);
        }
    }
}

/* if used directly, $fileopened will still be null, assuming user opened it; won't be closed later */
bool tiniwriter_to_file(tiniwriter_t* tw, FILE* hnd)
{
    tw->cb = wrap_to_file;
    tw->method = recv_to_file;
    tw->recv_handle = hnd;
    return true;
}

bool tiniwriter_to_filename(tiniwriter_t* tw, const char* file)
{
    FILE* hnd;
    if((hnd = fopen(file, "wb")) == NULL)
    {
        return false;
    }
    tw->fileopened = true;
    return tiniwriter_to_file(tw, hnd);
}

bool tiniwriter_write(tiniwriter_t* tw, const char* str, size_t slen)
{
    switch(tw->method)
    {
        case recv_to_file:
            if(tw->cb(tw, str, slen, NULL))
            {
                fflush(tw->recv_handle);
                return true;
            }
            break;
        default:
            fprintf(stderr, "error: assigned method is either unimplemented or undefined\n");
            break;
    }
    return false;
}

bool tiniwriter_section(tiniwriter_t* tw, const char* s)
{
    tw->currentsection = tw;
    if(!tiniwriter_write(tw, "[", 1))
    {
        return false;
    }
    if(!tiniwriter_write(tw, s, strlen(s)))
    {
        return false;
    }
    if(!tiniwriter_write(tw, "]", 1))
    {
        return false;
    }
    if(!tiniwriter_write(tw, "\n", 1))
    {
        return false;
    }
    return true;
}

bool tiniwriter_comment(tiniwriter_t* tw, const char* arg, ...)
{
    va_list ap;
    va_start(ap, arg);
    while(arg != NULL)
    {
        if(!tiniwriter_write(tw, "# ", 2))
        {
            va_end(ap);
            return false;
        }
        if(!tiniwriter_write(tw, arg, strlen(arg)))
        {
            va_end(ap);
            return false;
        }
        if(!tiniwriter_write(tw, "\n", 1))
        {
            va_end(ap);
            return false;
        }
        arg = va_arg(ap, const char*);
    }
    va_end(ap);
    return true;
}

bool tiniwriter_kv(tiniwriter_t* tw, const char* key, const char* fmt, ...)
{
    size_t sz;
    size_t needed;
    char* buf;
    va_list va;
    va_start(va, fmt);
    /* pcc doesn't declare vsnprintf properly - not an issue with clang, gcc, msvc, tcc, though. */
    #ifdef __PCC__
    needed = _vsnprintf(NULL, 0, fmt, va) + 1;
    #else
    needed = vsnprintf(NULL, 0, fmt, va) + 1;
    #endif
    if((buf = (char*)malloc(needed)) == NULL)
    {
        va_end(va);
        return false;
    }
    sz = vsprintf(buf, fmt, va);
    va_end(va);
    if(!tiniwriter_write(tw, key, strlen(key)))
    {
        return false;
    }
    if(!tiniwriter_write(tw, " = ", 3))
    {
        return false;
    }
    if(!tiniwriter_write(tw, buf, sz))
    {
        return false;
    }
    if(!tiniwriter_write(tw, "\n", 1))
    {
        return false;
    }
    return true;
}

int main()
{
    tiniwriter_t tw;
    tiniwriter_init(&tw);
    tiniwriter_to_filename(&tw, "out.ini");
    tiniwriter_section(&tw, "main");
        tiniwriter_comment(&tw, "login information", "blah blah", "etc", NULL);
        tiniwriter_kv(&tw, "user", "johndoe");
    tiniwriter_fini(&tw);
    printf("all done!\n");
}

