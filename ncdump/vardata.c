/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncdump/vardata.c,v 1.48 2010/05/05 22:15:39 dmh Exp $
 *********************************************************************/

#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netcdf.h>
#include "utils.h"
#include "netcdf.h"
#include "nccomps.h"
#include "dumplib.h"
#include "ncdump.h"
#include "indent.h"
#include "vardata.h"
#include "netcdf_aux.h"

/* If set, then print char variables as utf-8.
   If not set, then print non-printable characters as octal.
   The latter was the default before this change.
*/
#define UTF8CHARS

/* maximum len of string needed for one value of a primitive type */
#define MAX_OUTPUT_LEN 100

#define LINEPIND	"    "	/* indent of continued lines */

#define LBRACE "{"
#define RBRACE "}"

extern fspec_t formatting_specs; /* set from command-line options */

/* Only read this many values at a time, if last dimension is larger
  than this */
#define VALBUFSIZ 10000

static int linep;		/* line position, not counting global indent */
static int max_line_len;	/* max chars per line, not counting global indent */


/* set position in line before lput() calls */
static void
set_indent(int in) {
    linep = in;
}


void
set_max_len(int len) {
    max_line_len = len-2;
}


/*
 * Output a string that should not be split across lines.  If it would
 * make current line too long, first output a newline and current
 * (nested group) indentation, then continuation indentation, then
 * output string.  If string ends with a newline to force short line,
 * reset indentation after output.
 */
void
lput(const char *cp) {
    int nn = (int)strlen(cp);

    if (nn+linep > max_line_len && nn > 2) {
	(void) fputs("\n", stdout);
	indent_out();
	(void) fputs(LINEPIND, stdout);
	linep = (int)strlen(LINEPIND) + indent_get();
    }
    (void) fputs(cp,stdout);
    if (nn > 0 && cp[nn - 1] == '\n') {
	linep = indent_get();
    } else
	linep += nn;
}


/*--------------------------------------------------------------------------*/

/* Support function for print_att_times.
 * Output a string that should not be split across lines.
 * Keep track of position on print line.
 * Wrap print lines as needed to keep within requested line length.
 * Start CDL comment on each new line.
 * Handle line indentation.
 *
 * This function is like lput in vardata.c, with variations.
 * Hopefully this function will later be absorbed into a more
 * general lput-type function.
 */

#define CDL_COMMENT_PREFIX "// "
void
lput2(
    const char *cp,		/* string to print */
    bool_t first_item,		/* identify first item in list */
    bool_t wrap		/* line wrap control: true=enable,
    				 * false=stay on same line  */
    )
{
    static size_t linep;			/* current line position (number of */
    					/*   chars); saved between calls    */
    size_t len_prefix = strlen (CDL_COMMENT_PREFIX);
    bool_t make_newline;

    size_t len1 = strlen(cp);		/* length of input string */

    assert (len1 > 0);

/* (1) Single space or newline/indent sequence, as needed. */

    linep = linep + 1 + len1;		/* new line position, without newline */
    					/* add 1 extra for preceding space   */

    make_newline = (wrap && (first_item || linep > max_line_len + 2));
    					/* NEVER new line in no-wrap mode */

    if (make_newline) {			/* start new line, if needed */
        printf ("\n");
	indent_out();			/* same exact indentation as pr_att */
	printf ("\t\t");		/* (possible problem here) */
        printf ("  ");			/* add indent for CDL comment */
	linep = 16 + 2 + len1;		/* recompute new line position */
        				/* with newline + indents      */
    } else {
	printf (" ");			/* always one space, if not newline */
    }

/* (2) Add CDL comment prefix, if needed. */

    if (len_prefix > 0) {
	if (first_item || make_newline) {
	    printf (CDL_COMMENT_PREFIX);
            linep = linep + len_prefix;
	}
    }

/* (3) Output caller's string value. */

    printf ("%s", cp);
}


/*
 * Output a value of an attribute.
 */
static void
print_any_att_val (
    struct safebuf_t *sb,	/* string where output goes */
    const ncatt_t *attp,	/* attrbute */
    const void *valp		/* pointer to the value */
	    ) {
    nctype_t *typ = attp->tinfo;
    (*typ->typ_tostring)(typ, sb, valp);
}


/*
 * Output a value of a variable, except if there is a fill value for
 * the variable and the value is the fill value, print the fill-value string
 * instead.  (Floating-point fill values need only be within machine epsilon of
 * defined fill value.)
 */
static void
print_any_val(
    safebuf_t *sb,		/* string where output goes */
    const ncvar_t *varp,	/* variable */
    const void *valp		/* pointer to the value */
	    )
{
    if (varp->has_fillval &&
	(*(varp->tinfo->val_equals))((const nctype_t *)varp->tinfo,
				     (const void*)varp->fillvalp, valp) ) {
	sbuf_cpy(sb, FILL_STRING);
    } else {
	(*varp->val_tostring)(varp, sb, valp);
    }
}

/*
 * print last delimiter in each line before annotation (, or ;)
 */
static void
lastdelim (bool_t more, bool_t lastrow)
{
    if (more) {
	printf(", ");
    } else {
	if(lastrow) {
	    printf(";");
	} else {
	    printf(",");
	}
    }
}

/*
 * print last delimiter in each line before annotation (, or ;)
 */
static void
lastdelim2 (bool_t more, bool_t lastrow)
{
    if (more) {
	lput(", ");
    } else {
	if(lastrow) {
	    lput(" ;");
	    lput("\n");
	} else {
	    lput(",\n");
	    lput("  ");
	}
    }
}


/*
 * Print a number of attribute values
 */
void
pr_any_att_vals(
     const ncatt_t *ap,		/* attribute */
     const void *vals		/* pointer to block of values */
     )
{
    size_t iel;
    size_t len = ap->len;		/* number of values to print */
    const char *valp = (const char *)vals;
    safebuf_t *sb = sbuf_new();

    for (iel = 0; iel < len - 1; iel++) {
	print_any_att_val(sb, ap, (void *)valp);
	valp += ap->tinfo->size; /* next value according to type */
	sbuf_cat(sb, iel == len - 1 ? "" : ", ");
	lput(sbuf_str(sb));
    }
    print_any_att_val(sb, ap, (void *)valp);
    lput(sbuf_str(sb));
    sbuf_free(sb);
}

/*
 * Prints brief annotation for a row of data values
 */
static void
annotate_brief(
    const ncvar_t *vp,		/* variable */
    const size_t *cor,		/* corner coordinates */
    size_t vdims[]		/* variable dimension sizes */
    )
{
    int vrank = vp->ndims;
    int id;
    printf ("// ");
    print_name(vp->name);
    printf("(");

    switch (formatting_specs.data_lang) {
    case LANG_C:
	/* print brief comment with C variable indices */
	for (id = 0; id < vrank-1; id++)
	    printf("%lu,", (unsigned long)cor[id]);
	if (vdims[vrank-1] == 1)
	    printf("0");
	else
	    printf(" 0-%lu", (unsigned long)vdims[vrank-1]-1);
	break;
    case LANG_F:
	/* print brief comment with Fortran variable indices */
	if (vdims[vrank-1] == 1)
	    printf("1");
	else
	    printf("1-%lu ", (unsigned long)vdims[vrank-1]);
	for (id = vrank-2; id >=0 ; id--) {
	    printf(",%lu", (unsigned long)(1 + cor[id]));
	}
	break;
    }
    printf(")\n");
    indent_out();
    printf("    ");
    set_indent(4 + indent_get());
}

/*
 * Annotates a value in data section with var name and indices in comment
 */
static void
annotate(
     const ncvar_t *vp,		/* variable */
     const size_t *cor,		/* corner coordinates */
     long iel			/* which element in current row */
     )
{
    int vrank = vp->ndims;
    int id;

    /* print indices according to data_lang */
/*     printf("  // %s(", vp->name); */
    printf("  // ");
    print_name(vp->name);
    printf("(");
    switch (formatting_specs.data_lang) {
      case LANG_C:
	/* C variable indices */
	for (id = 0; id < vrank-1; id++)
	  printf("%lu,", (unsigned long) cor[id]);
	printf("%lu", (unsigned long) cor[id] + (unsigned long) iel);
	break;
      case LANG_F:
	/* Fortran variable indices */
	printf("%lu", (unsigned long) cor[vrank-1] + (unsigned long) iel + 1);
	for (id = vrank-2; id >=0 ; id--) {
	    printf(",%lu", 1 + (unsigned long) cor[id]);
	}
	break;
    }
    printf(")\n    ");
}

/*
 * Print a number of char variable values as a text string, where the
 * optional comments for each value identify the variable, and each
 * dimension index.
 */
static void
pr_tvals(
     const ncvar_t *vp,		/* variable */
     size_t len,		/* number of values to print */
     const char *vals,		/* pointer to block of values */
     const size_t *cor		/* corner coordinates */
     )
{
    long iel;
    const char *sp;

    printf("\"");
    /* adjust len so trailing nulls don't get printed */
    sp = vals + len;
    while (len != 0 && *--sp == '\0')
	len--;
    /* Walk the sequence of characters and write control characters in escape form. */
    for (iel = 0; iel < len; iel++) {
	unsigned char uc;
	switch (uc = (unsigned char)(*vals++ & 0377)) {
	case '\b':
	    printf("\\b");
	    break;
	case '\f':
	    printf("\\f");
	    break;
	case '\n':	/* generate linebreaks after new-lines */
	    printf("\\n\",\n    \"");
	    break;
	case '\r':
	    printf("\\r");
	    break;
	case '\t':
	    printf("\\t");
	    break;
	case '\v':
	    printf("\\v");
	    break;
	case '\\':
	    printf("\\\\");
	    break;
	case '\'':
	    printf("\\\'");
	    break;
	case '\"':
	    printf("\\\"");
	    break;
	default:
#ifdef UTF8CHARS
	    if (!isprint(uc))
		printf("\\%.3o",uc);
	    else
#endif /*UTF8CHARS*/
		printf("%c",uc);
	    break;
	}
    }
    printf("\"");
    /* if (formatting_specs.full_data_cmnts) { */
    /* 	lastdelim (0, lastrow); */
    /* 	annotate (vp,  (size_t *)cor, 0L); */
    /* }  */
}


/*
 * Updates a vector of ints, odometer style.  Returns 0 if odometer
 * overflowed, else 1.
 */
static int
upcorner(
     const size_t *dims,	/* The "odometer" limits for each dimension */
     int ndims,			/* Number of dimensions */
     size_t* odom,		/* The "odometer" vector to be updated */
     const size_t* add		/* A vector to "add" to odom on each update */
     )
{
    int id;
    int ret = 1;

    for (id = ndims-1; id > 0; id--) {
	odom[id] += add[id];
	if(odom[id] >= dims[id]) {
	    odom[id-1]++;
	    odom[id] -= dims[id];
	}
    }
    odom[0] += add[0];
    if (odom[0] >= dims[0])
      ret = 0;
    return ret;
}

/*  Print data values for variable varid.
 *
 * Recursive to handle possibility of variables with multiple
 * unlimited dimensions, for which the CDL syntax requires use of "{"
 * and "}" in data section to disambiguate the size of nested records
 * in a simple linear list of values.
 */
static int
print_rows(
    int level,          /* 0 at top-level, incremented for each recursive level */
    int ncid,		/* netcdf id */
    int varid,		/* variable id */
    const ncvar_t *vp,	/* variable */
    size_t vdims[],    	/* variable dimension sizes */
    size_t cor[],      	/* corner coordinates */
    size_t edg[],      	/* edges of hypercube */
    void *vals,   	/* allocated buffer for ncols values in a row */
    int marks_pending	/* number of pending closing "}" record markers */
    )
{
    int rank = vp->ndims;
    size_t ncols = rank > 0 ? vdims[rank - 1] : 1; /* number of values in a row */
    int d0 = 0;
    int i;
    bool_t mark_record = (level > 0 && is_unlim_dim(ncid, vp->dims[level]));
    safebuf_t *sb = sbuf_new();
    if (rank > 0)
	d0 = (int)vdims[level];
    if(mark_record) { /* the whole point of this recursion is printing these "{}" */
	lput(LBRACE);
	marks_pending++;	/* matching "}"s to emit after last "row" */
    }
    if(rank - level > 1) {     	/* this level is just d0 next levels */
	size_t *local_cor = emalloc((size_t)(rank + 1) * sizeof(size_t));
	size_t *local_edg = emalloc((size_t)(rank + 1) * sizeof(size_t));
	for(i = 0; i < rank; i++) {
	    local_cor[i] = cor[i];
	    local_edg[i] = edg[i];
	}
	local_cor[level] = 0;
	local_edg[level] = 1;
	for(i = 0; i < d0 - 1; i++) {
	    print_rows(level + 1, ncid, varid, vp, vdims,
		       local_cor, local_edg, vals, 0);
	    local_cor[level] += 1;
	}
	print_rows(level + 1, ncid, varid, vp, vdims,
		   local_cor, local_edg, vals, marks_pending);
	free(local_edg);
	free(local_cor);
    } else {			/* bottom out of recursion */
	char *valp = vals;
	bool_t lastrow;
	int j;
	if(formatting_specs.brief_data_cmnts && rank > 1 && ncols > 0) {
	    annotate_brief(vp, cor, vdims);
	}
	NC_CHECK(nc_get_vara(ncid, varid, cor, edg, (void *)valp));

	/* Test if we should treat array of chars as strings along last dimension  */
	if(vp->type == NC_CHAR && (vp->fmt == 0 || NCSTREQ(vp->fmt,"%s") || NCSTREQ(vp->fmt,""))) {
	    pr_tvals(vp, ncols, vals, cor);
	} else {			/* for non-text variables */
	    for(i=0; i < d0 - 1; i++) {
		print_any_val(sb, vp, (void *)valp);
		valp += vp->tinfo->size; /* next value according to type */
		if (formatting_specs.full_data_cmnts) {
		    printf("%s, ", sb->buf);
		    annotate (vp, cor, i);
		} else {
		    sbuf_cat(sb, ", ");
		    lput(sbuf_str(sb));
		}
	    }
	    print_any_val(sb, vp, (void *)valp);
	}
        /* In case vals has memory hanging off e.g. vlen or string, make sure to reclaim it */
        NC_CHECK(nc_reclaim_data(ncid,vp->type,vals,ncols));

	/* determine if this is the last row */
	lastrow = true;
	for(j = 0; j < rank - 1; j++) {
            if (cor[j] != vdims[j] - 1) {
		lastrow = false;
		break;
            }
	}
	if (formatting_specs.full_data_cmnts) {
      for (j = 0; j < marks_pending; j++) {
		sbuf_cat(sb, RBRACE);
      }
      printf("%s", sbuf_str(sb));
      lastdelim (0, lastrow);
      annotate (vp, cor, (d0 > 0 ? d0-1 : d0));
	} else {
            for (j = 0; j < marks_pending; j++) {
		sbuf_cat(sb, RBRACE);
            }
      	    lput(sbuf_str(sb));
            lastdelim2 (0, lastrow);
	}
    }
    sbuf_free(sb);
    return NC_NOERR;
}

/* Output the data for a single variable, in CDL syntax. */
int
vardata(
     const ncvar_t *vp,		/* variable */
     size_t vdims[],		/* variable dimension sizes */
     int ncid,			/* netcdf id */
     int varid			/* variable id */
     )
{
    size_t *cor;	     /* corner coordinates */
    size_t *edg;	     /* edges of hypercube */
    size_t *add;	     /* "odometer" increment to next "row"  */
    void *vals;

    int id;
    size_t ncols;
    int vrank = vp->ndims;

    int level = 0;
    int marks_pending = 0;

    cor = (size_t *) emalloc((size_t)(1 + vrank) * sizeof(size_t));
    edg = (size_t *) emalloc((size_t)(1 + vrank) * sizeof(size_t));
    add = (size_t *) emalloc((size_t)(1 + vrank) * sizeof(size_t));

    if(vrank == 0) { /*scalar*/
	cor[0] = 0;
	edg[0] = 1;
    } else {
	for (id = 0; id < vrank; id++) {
	    cor[id] = 0;
	    edg[id] = 1;
	}
    }
    printf("\n");
    indent_out();
    printf(" ");
    print_name(vp->name);
    if (vrank <= 1) {
	printf(" = ");
	set_indent ((int)strlen(vp->name) + 4 + indent_get());
    } else {
	printf(" =\n  ");
	set_indent (2 + indent_get());
    }

    if (vrank == 0) {
	ncols = 1;
    } else {
	ncols = vdims[vrank-1];	/* size of "row" along last dimension */
	edg[vrank-1] = ncols;
	for (id = 0; id < vrank; id++)
	  add[id] = 0;
	if (vrank > 1)
	  add[vrank-2] = 1;
    }
    vals = emalloc(ncols * vp->tinfo->size);

    NC_CHECK(print_rows(level, ncid, varid, vp, vdims, cor, edg, vals, marks_pending));
    free(vals);
    free(cor);
    free(edg);
    free(add);

    return 0;
}


/*
 * print last delimiter in each line before annotation (, or ;)
 */
static void
lastdelim2x (bool_t more, bool_t lastrow)
{
    if (more) {
	lput(" ");
    } else {
	if(lastrow) {
	    lput("\n   ");
	} else {
	    lput("\n     ");
	}
    }
}


/*
 * Print a number of char variable values as a text string for NcML
 */
static void
pr_tvalsx(
     const ncvar_t *vp,		/* variable */
     size_t len,		/* number of values to print */
     bool_t more,		/* true if more data for this row will
				 * follow, so add trailing comma */
     bool_t lastrow,		/* true if this is the last row for this
				 * variable, so terminate with ";" instead
				 * of "," */
     const char *vals		/* pointer to block of values */
     )
{
    long iel;
    const char *sp;

    printf("\"");
    /* adjust len so trailing nulls don't get printed */
    sp = vals + len;
    while (len != 0 && *--sp == '\0')
	len--;
    for (iel = 0; iel < len; iel++) {
	unsigned char uc;
	switch (uc = (unsigned char)(*vals++ & 0377)) {
	case '\b':
	    printf("\\b");
	    break;
	case '\f':
	    printf("\\f");
	    break;
	case '\n':	/* generate linebreaks after new-lines */
	    printf("\\n\",\n    \"");
	    break;
	case '\r':
	    printf("\\r");
	    break;
	case '\t':
	    printf("\\t");
	    break;
	case '\v':
	    printf("\\v");
	    break;
	case '\\':
	    printf("\\\\");
	    break;
	case '\'':
	    printf("\\\'");
	    break;
	case '\"':
	    printf("\\\"");
	    break;
	default:
	    if (isprint(uc))
		printf("%c",uc);
	    else
		printf("\\%.3o",uc);
	    break;
	}
    }
    printf("\"");
    lastdelim2x (more, lastrow);
}


/*
 * Print a number of variable values for NcML
 */
static void
pr_any_valsx(
     const ncvar_t *vp,		/* variable */
     size_t len,		/* number of values to print */
     bool_t more,		/* true if more data for this row will
				 * follow, so add trailing comma */
     bool_t lastrow,		/* true if this is the last row for this
				 * variable, so terminate with ";" instead
				 * of "," */
     const void *vals		/* pointer to block of values */
     )
{
    long iel;
    safebuf_t *sb = sbuf_new();
    const char *valp = (const char *)vals;

    for (iel = 0; iel < len-1; iel++) {
	print_any_val(sb, vp, (void *)valp);
	valp += vp->tinfo->size; /* next value according to type */
	sbuf_cat(sb, " ");
	lput(sbuf_str(sb));
    }
    print_any_val(sb, vp, (void *)valp);
    lput(sbuf_str(sb));
    lastdelim2x (more, lastrow);
    sbuf_free(sb);
}


/* Output the data for a single variable, in NcML syntax.
 *  TODO: currently not called, need option for NcML with values ... */
int
vardatax(
     const ncvar_t *vp,		/* variable */
     size_t vdims[],		/* variable dimension sizes */
     int ncid,			/* netcdf id */
     int varid			/* variable id */
     )
{
    size_t *cor;	     /* corner coordinates */
    size_t *edg;	     /* edges of hypercube */
    size_t *add;	     /* "odometer" increment to next "row"  */
    void *vals;

    int id;
    int ir;
    size_t nels;
    size_t ncols;
    size_t nrows;
    int vrank = vp->ndims;

    cor = (size_t *) emalloc((size_t)(vrank + 1) * sizeof(size_t));
    edg = (size_t *) emalloc((size_t)(vrank + 1) * sizeof(size_t));
    add = (size_t *) emalloc((size_t)(vrank + 1) * sizeof(size_t));

    nels = 1;
    for (id = 0; id < vrank; id++) {
	cor[id] = 0;
	edg[id] = 1;
	nels *= vdims[id];	/* total number of values for variable */
    }

    printf("    <values>\n     ");
    set_indent (7);

    if (vrank < 1) {
	ncols = 1;
    } else {
	ncols = vdims[vrank-1];	/* size of "row" along last dimension */
	edg[vrank-1] = vdims[vrank-1];
	for (id = 0; id < vrank; id++)
	  add[id] = 0;
	if (vrank > 1)
	  add[vrank-2] = 1;
    }
    nrows = nels/ncols;		/* number of "rows" */
    vals = emalloc(ncols * vp->tinfo->size);

    for (ir = 0; ir < nrows; ir++) {
	size_t corsav = 0;
	bool_t lastrow;

	if (vrank > 0) {
	    corsav = cor[vrank-1];
	}
	lastrow = (bool_t)(ir == nrows-1);

	if (vrank > 0)
	    edg[vrank-1] = ncols;
	NC_CHECK(nc_get_vara(ncid, varid, cor, edg, vals) );
	/* Test if we should treat array of chars as a string  */
	if(vp->type == NC_CHAR &&
	   (vp->fmt == 0 || NCSTREQ(vp->fmt,"%s") || NCSTREQ(vp->fmt,""))) {
	    pr_tvalsx(vp, ncols, 0, lastrow, (char *) vals);
	} else {
	    pr_any_valsx(vp, ncols, 0, lastrow, vals);
	}

	if (vrank > 0)
	    cor[vrank-1] += ncols;

	if (vrank > 0)
	  cor[vrank-1] = corsav;
	if (ir < nrows-1)
	  if (!upcorner(vdims,vp->ndims,cor,add))
	    error("vardata: odometer overflowed!");
	set_indent(2);
    }
    printf(" </values>\n");
    free(vals);
    free(cor);
    free(edg);
    free(add);
    return 0;
}
