/*
 * A safer saner malloc, for careless programmers
 * $Revision: 7.16 $
 */

#include "sc.h"

void		fatal();

#define	MAGIC	(double)1234567890.12344

char *
scxmalloc(unsigned n)
{
	register char *ptr;

	if ((ptr = malloc(n + sizeof(double))) == NULL)
		fatal("scxmalloc: no memory");
	*((double *) ptr) = MAGIC;		/* magic number */
	return(ptr + sizeof(double));
}

/* we make sure realloc will do a malloc if needed */
char *
scxrealloc(char *ptr, unsigned n)
{
	if (ptr == NULL)
		return(scxmalloc(n));

	ptr -= sizeof(double);
	if (*((double *) ptr) != MAGIC)
		fatal("scxrealloc: storage not scxmalloc'ed");

	if ((ptr = realloc(ptr, n + sizeof(double))) == NULL)
		fatal("scxmalloc: no memory");
	*((double *) ptr) = MAGIC;		/* magic number */
	return(ptr + sizeof(double));
}

void
scxfree(char *p)
{
	if (p == NULL)
		fatal("scxfree: NULL");
	p -= sizeof(double);
	if (*((double *) p) != MAGIC)
		fatal("scxfree: storage not malloc'ed");
	free(p);
}

#ifdef PSC
void
fatal(char *str)
{
    fprintf(stderr,"%s\n", str);
    exit(1);
}
#else
void
fatal(char *str)
{
    deraw(1);
    fprintf(stderr,"%s\n", str);
    diesave();
    exit(1);
}
#endif /* PSC */
