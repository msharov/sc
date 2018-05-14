// SC is free software distributed under the MIT license

#include "sc.h"

void* scxmalloc (unsigned n)
{
    void* p = malloc (n);
    assert (p && "out of memory");
    if (!p) abort();
    return p;
}

// we make sure realloc will do a malloc if needed
void* scxrealloc (void* op, unsigned n)
{
    void* p = realloc (op, n);
    assert (p && "out of memory");
    if (!p) abort();
    return p;
}

void scxfree (void *p)
{
    assert (p && "scxfree: NULL");
    free(p);
}
