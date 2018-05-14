// SC is free software distributed under the MIT license

#include "sc.h"

static struct abbrev* _abbr_base;

static bool are_abbrevs (void)
{
    return _abbr_base != NULL;
}

void add_abbr (char *string)
{
    if (!string || *string == '\0') {
	if (!are_abbrevs()) {
	    error ("No abbreviations defined");
	    return;
	} else {
	    const char* pager = getenv("PAGER");
	    if (!pager)
		pager = DFLT_PAGER;

	    char px[MAXCMD];
	    snprintf (px, sizeof(px), "|%s", pager);

	    pid_t pid;
	    FILE* f = openfile (px, &pid, NULL);
	    if (!f) {
		error ("Can't open pipe to %s", pager);
		return;
	    }

	    fprintf(f, "\nAbbreviation    Expanded"
		       "\n------------    --------");
	    struct abbrev* a = _abbr_base;
	    for (struct abbrev* nexta = _abbr_base; nexta; a = nexta, nexta = a->a_next) {}
	    while (a) {
		fprintf(f, "%-15s %s\n", a->abbr, a->exp);
		a = a->a_prev;
	    }
	    closefile(f, pid, 0);
	    return;
	}
    }

    char* expansion = strchr (string, ' ');
    if (expansion)
	*expansion++ = '\0';

    if (isalpha(*string) || isdigit(*string) || *string == '_') {
	for (const char* p = string; *p; ++p) {
	    if (!isalpha(*p) && !isdigit(*p) && *p != '_') {
		error ("Invalid abbreviation: %s", string);
		scxfree (string);
		return;
	    }
	}
    } else {
	for (const char* p = string; *p; ++p) {
	    if ((isalpha(*p) || isdigit(*p) || *p == '_') && p[1]) {
		error ("Invalid abbreviation: %s", string);
		scxfree (string);
		return;
	    }
	}
    }
    
    struct abbrev *prev = NULL;
    if (!expansion) {
	struct abbrev* a = find_abbr(string, strlen(string), &prev);
	if (a) {
	    error("abbrev \"%s %s\"", a->abbr, a->exp);
	    return;
	} else {
	    error("abreviation \"%s\" doesn't exist", string);
	    return;
	}
    }
 
    if (find_abbr(string, strlen(string), &prev))
	del_abbr(string);

    struct abbrev* a = scxmalloc (sizeof(struct abbrev));
    a->abbr = string;
    a->exp = expansion;

    if (prev) {
	a->a_next = prev->a_next;
	a->a_prev = prev;
	prev->a_next = a;
	if (a->a_next)
	    a->a_next->a_prev = a;
    } else {
	a->a_next = _abbr_base;
	a->a_prev = NULL;
	if (_abbr_base)
	    _abbr_base->a_prev = a;
	_abbr_base = a;
    }
}

void del_abbr (const char *abbrev)
{
    struct abbrev** prev = NULL;
    struct abbrev* a = find_abbr (abbrev, strlen(abbrev), prev);
    if (!a)
	return;

    if (a->a_next)
        a->a_next->a_prev = a->a_prev;
    if (a->a_prev)
        a->a_prev->a_next = a->a_next;
    else
	_abbr_base = a->a_next;
    scxfree (a->abbr);
    scxfree (a);
}

struct abbrev* find_abbr (const char* abbrev, int len, struct abbrev** prev)
{
    bool exact = true;
    if (len < 0) {
	exact = false;
	len = -len;
    }
    for (struct abbrev* a = _abbr_base; a; a = a->a_next) {
	int cmp = strncmp (abbrev, a->abbr, len);
	if (cmp > 0)
	    break;
	if (!cmp && (!exact || strlen(a->abbr) == (unsigned)len))
	    return (a);
	*prev = a;
    }
    return NULL;
}
