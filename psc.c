// SC is free software distributed under the MIT license

// Sc parse routine
//
// usage psc options
// options:
//   -L		Left justify strings.  Default is right justify.
//   -r		Assemble data into rows first, not columns.
//   -R	n	Increment by n between rows 
//   -C n	Increment by n between columns
//   -n n	Length of the row (column) should be n.
//   -s v	Top left location in the spreadsheet should be v; eg, k5
//   -d c       Use c as the delimiter between the fields.
//   -k         Keep all delimiters - Default is strip multiple delimiters to 1.
//   -f         suppress 'format' lines in output
//   -S		Use strings vs numbers for numbers
//   -P		Use numbers only when there is no [-+eE] (plain numbers only)
//
//  Author: Robert Bond
//  Adjustments: Jeff Buhrt, Eric Putz and Chuck Martin

#include "sc.h"

enum { END, NUMBER, ALPHA, SPACE, EOL };

extern int psc_growtbl (int rowcol, int topcol);
static int scan (void);
static int getcol (char *p);
static int getrow (char *p);
static char* pcoltoa (int col);

int *fwidth;
int *precision;
int maxcols;
int *realfmt;

int curlen;
int curcol, coff;
int currow, roff;
int first;
int effr, effc;

// option flags reset
int colfirst = false;
int leftadj = false;
int r0 = 0;
int c0 = 0;
int rinc = 1;
int cinc = 1;
int len = 20000;
char delim1 = ' ';
char delim2 = '\t';
int strip_delim = true;
int drop_format = false;
int strnums	= false;
int plainnums	= false;
int maxrows = 0, maxcols = 0;

char token [1000];

int main (int argc, char** argv)
{
    int c;
    int i, j;
    char *p;

    while ((c = getopt(argc, argv, "rfLks:R:C:n:d:SPv")) != EOF) {
	switch (c) {
	    case 'r': colfirst = true; break;
	    case 'L': leftadj = true; break;
	    case 's': c0 = getcol(optarg); r0 = getrow(optarg); break;
	    case 'R': rinc = atoi(optarg); break;
	    case 'C': cinc = atoi(optarg); break;
	    case 'n': len = atoi(optarg); break;
	    case 'd': delim1 = optarg[0]; delim2 = '\0'; break;
	    case 'k': strip_delim = false; break;
	    case 'f': drop_format = true; break;
	    case 'S': strnums = true; break;
	    case 'P': plainnums = true; break;
	    case 'v': fprintf(stderr,"%s: " SC_VERSTRING "\n", argv[0]); // fallthrough
	    default:
		fprintf(stderr,"Usage: %s [-rkfLSPv] [-s v] [-R i] [-C i] [-n i] [-d c]\n", argv[0]);
		exit(1);
        }
    }
    if (optind < argc) {
	fprintf(stderr,"Usage: %s [-rL] [-s v] [-R i] [-C i] [-n i] [-d c]\n", argv[0]);
	exit(1);
    }
    // setup the spreadsheet arrays
    if (!psc_growtbl(GROWNEW, 0))
	exit(1);

    curlen = 0;
    curcol = c0; coff = 0;
    currow = r0; roff = 0;
    first = true;

    while (1) {
	effr = currow+roff;
	effc = curcol+coff;
	switch (scan()) {
	case END:
	    if (drop_format) exit(0);
	    for (i = 0; i<maxcols; i++) {
		if (fwidth[i])
		    printf("format %s %d %d %d\n", pcoltoa(i), fwidth[i]+1, precision[i], REFMTFIX);
	    }
	    exit(0);
	case NUMBER:
	    first = false;
	    printf("let %s%d = %s\n", pcoltoa(effc), effr, token);
	    if (effc >= maxcols - 1) {
		if (!psc_growtbl(GROWCOL, effc)) {
		    fprintf(stderr, "Invalid column used: %s\n", pcoltoa(effc));
		    continue;
		}
	    }
	    i = 0;
	    j = 0;
	    p = token;
	    while (*p && *p != '.') {
		p++; i++;
	    }
	    if (*p) {
		p++; i++;
	    }
	    while (*p) {
		p++; i++; j++;
	    }
	    {   int	ow, nw;

		ow = fwidth[effc] - precision[effc];
		if (precision[effc] < j)
			precision[effc] = j;
	
		if (fwidth[effc] < i)
			fwidth[effc] = i;

		// now make sure:
		//	1234.567890 (format 11 6)
		//	1234567.890 (format 11 3)
		//	both show (format 14 6)
		//		(really it uses 15 6 to separate columns)
		if ((nw = i - j) > ow)
			fwidth[effc] += nw - (fwidth[effc] - precision[effc]);
	    }
	    break;
	case ALPHA:
	    first = false;
	    if (leftadj)
		printf("leftstring %s%d = \"%s\"\n", pcoltoa(effc),effr,token); 
	    else
		printf("rightstring %s%d = \"%s\"\n",pcoltoa(effc),effr,token); 
	    if (effc >= maxcols - 1) {
		if (!psc_growtbl(GROWCOL, effc)) {
		    fprintf(stderr, "Invalid column used: %s\n", pcoltoa(effc));
		    continue;
		}
	    }
	    i = strlen(token);
	    if (i > fwidth[effc])
		fwidth[effc] = i;
	    break;
	case SPACE:
	    if (first && strip_delim)
		break;
	    if (colfirst)
		roff++;
	    else
		coff++;
	    break;
	case EOL:
	    curlen++;
	    roff = 0;
	    coff = 0;
	    first = true;
	    if (colfirst) {
		if (curlen >= len) {
		    curcol = c0;
		    currow += rinc;
		    curlen = 0;
		} else {
		    curcol += cinc;
		}
	    } else {
		if (curlen >= len) {
		    currow = r0;
		    curcol += cinc;
		    curlen = 0;
		} else {
		    currow += rinc;
		}
	    }
	    break;
	}
    }
}

static int scan (void)
{
    int c;
    char *p;
    int founddigit;

    p = token;
    c = getchar();

    if (c == EOF)
	return END;

    if (c == '\n')
	return EOL;

    if (c == delim1 || c == delim2) {
        if (strip_delim) {
	    while ((c = getchar()) && (c == delim1 || c == delim2)) {}
	    ungetc(c, stdin);
	} 
	return SPACE;
    }

    if (c == '\"') {
	while ((c = getchar()) && c != '\"' && c != '\n' && c != EOF)
	    *p++ = c;
	if (c != '\"')
	    ungetc(c, stdin);
	*p = '\0';
	return ALPHA;
    }

    while (c != delim1 && c != delim2 && c!= '\n' && c != EOF) {
	*p++ = c;
	c = getchar();
    }
    *p = '\0';
    ungetc(c, stdin);

    p = token;
    c = *p;
    founddigit = false;
    // str_nums always returns numbers as strings
    // plainnums returns 'numbers' with [-+eE] in them as strings
    // lastprtnum makes sure a number ends in one of [0-9eE.]
    if (!strnums && (isdigit(c) || c == '.' || c == '-' || c == '+')) {
	bool lastprtnum = false;
	while (isdigit(c) || c == '.' || (!plainnums && (c == '-' || c == '+' || c == 'e' || c == 'E'))) {
	    if (isdigit(c)) {
		lastprtnum = true;
		founddigit = true;
	    } else if (!(c == '.' || c == 'e' || c == 'E'))
		lastprtnum = false;
	    c = *p++;
	}
	if (c == '\0' && founddigit && lastprtnum)
	    return NUMBER;
	else
	    return ALPHA;
    }
    return ALPHA;
}
    
// turns [A-Z][A-Z] into a number
static int getcol (char *p)
{
    int col = 0;
    if (!p)
	return 0;
    while (*p && !isalpha(*p)) 
	++p;
    if (!*p)
	return 0;
    col = (toupper(*p) - 'A');
    if (isalpha(*++p)) 
	col = (col + 1)*26 + (toupper(*p) - 'A');
    return col;
}

// given a string turn it into a row number
static int getrow (char *p)
{
    int row = 0;
    if (!p)
	return 0;
    while (*p && !isdigit(*p))
	++p; 
    if (!*p)
	return 0;
    while (*p && isdigit(*p)) {
	row = row * 10 + *p - '0';
	++p;
    }
    return row;
}

// turns a column number into [A-Z][A-Z]
static char* pcoltoa (int col)
{
    static char rname[3];
    char *p = rname;

    if (col < 0 || col > 27*26)	// A-Z, AA-ZZ
	fprintf(stderr,"coltoa: invalid col: %d", col);

    if (col > 25) {
	*p++ = col/26 + 'A' - 1;
	col %= 26;
    }
    *p++ = col+'A';
    *p = '\0';
    return rname;
}
