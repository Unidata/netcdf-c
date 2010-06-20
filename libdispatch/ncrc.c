/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information.
*/
/*$Id$*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ncrc.h"

static int ncrcdebug = 0;

#define ALLOCINCR 16
/* mnemonic */
#define OPTIONAL 1

#define INDENTCHUNK "  "

static int initialized = 0;

static char hexchars[17] = "01234567890abcdef";

static char delims[] = "{}[]:,;";

/* Token types */
#define LBRACE '{'
#define RBRACE '}'
#define LBRACK '['
#define RBRACK ']'
#define COMMA ','
#define COLON ':'
#define EQUAL '='
#define SEMICOLON ';'
#define _ILLEGAL ncrc_unknown
#define _STRING ncrc_string
#define _NUMBER ncrc_number
#define _TRUE ncrc_true
#define _FALSE ncrc_false
#define _NULL ncrc_null

/* For debugging */
enum nonterms { _value=0, _map=1, _array=2, _pair=3, _const=4};
static char* nontermnames[] = {"value","map","array","pair","const"};

#define ENTER(proc) {if(ncrcdebug) trace(proc,0,0);}
#define LEAVE(proc,tf) {if(ncrcdebug) trace(proc,1,tf);}

#define FAIL(lexer,msg) do {(lexer)->errmsg = (msg); goto fail;} while(0)

/* Define static pre-defined nodes */
#define CONSTNODE(name) static ncrcnode ncrc_constant_##name = {ncrc_const,ncrc_##name}; ncrcnode* ncrc_const_##name = &ncrc_constant_##name

CONSTNODE(true);
CONSTNODE(false);
CONSTNODE(null);

/**************************************************/

typedef struct  ncrclist {
    ncrcnode** contents;
    size_t len; /* |text| */
    size_t alloc; /* |text| */
} ncrclist;

struct ncrctext {
    char* text;
    size_t len; /* |text| */
    size_t alloc; /* |text| */
    int pushback[2]; /* max pushback needed */
};

typedef struct ncrclexer {
    FILE* input;
    int token;    
    struct ncrctext text;
    int pushedback; /* 1=>keep current token */
    int lineno;
    int charno;
    char* errmsg;
} ncrclexer;

/****************************************/
static int value(ncrclexer* lexer, ncrcnode** nodep);
static int map(ncrclexer* lexer, ncrcnode** nodep, int);
static int pair(ncrclexer* lexer, ncrcnode** nodep);
static int array(ncrclexer* lexer, ncrcnode** nodep);
static int makeconst(ncrclexer* lexer, ncrcnode** nodep);
static ncrcnode* createncrcnode(ncrclexer* lexer,ncrcclass);

static int nexttoken(ncrclexer* lexer);
static void pushtoken(ncrclexer* lexer);
static unsigned int tohex(int c);
static void dumptoken(ncrclexer* lexer, int);
static int removeescapes(ncrclexer* lexer);

static void textclear(struct ncrctext* lexer);
static int textadd(struct ncrctext* text, int c);
static int textterminate(struct ncrctext* text);

static void pushback(ncrclexer* lexer, int c);
static int unpush(ncrclexer* lexer);
static int readc(ncrclexer* lexer);

static int listadd(ncrclist* list, ncrcnode* node);
static void listclear(ncrclist* list);

static void trace(enum nonterms nt, int leave, int ok);

/**************************************************/
int
ncrc(FILE* src, ncrcnode** nodep, ncrcerror* err)
{
    ncrclexer lexer;
    ncrcnode* node = NULL;
    int token;

    memset((void*)&lexer,0,sizeof(ncrclexer));

    if(!initialized) {
	ncrc_constant_true.constvalue = "true";
	ncrc_constant_true.constvalue = "false";
	ncrc_constant_true.constvalue = "null";
	initialized = 1;
    }
    lexer.input = src;
    lexer.lineno = 1;
    lexer.charno = 1;
    lexer.errmsg = NULL;

    token = nexttoken(&lexer);    

    /* Make braces optional at top level */
    if(token == LBRACE) {
        if(!map(&lexer,&node,!OPTIONAL)) goto fail;
    } else {
	pushtoken(&lexer);
        if(!map(&lexer,&node,OPTIONAL)) goto fail;	    
    }
    if(nodep) *nodep = node;
    return 1;

fail:
    if(err) {
	err->lineno = lexer.lineno;
	err->charno = lexer.charno-1;
	err->errmsg = lexer.errmsg;
    }
    if(node) ncrc_nodefree(node);
    return 0;
}

static int
value(ncrclexer* lexer, ncrcnode** nodep)
{
    ncrcnode* node = NULL;
    int token = 0;

    ENTER(_value);

    token = nexttoken(lexer);
    switch (lexer->token) {
    case LBRACE:
	if(!map(lexer,&node,!OPTIONAL)) goto fail;
	break;
    case LBRACK:
	if(!array(lexer,&node)) goto fail;
	break;
    case _STRING:
    case _NUMBER:
    case _NULL:
    case _TRUE:
    case _FALSE:
	pushtoken(lexer);
	if(!makeconst(lexer,&node)) goto fail;
	break;
    case _ILLEGAL:
    default: goto fail;
    }
    if(nodep) *nodep = node;

    LEAVE(_value,1);
    return 1;
fail:
    LEAVE(_value,0);
    return 0;
}

static int
map(ncrclexer* lexer, ncrcnode** nodep, int optional)
{
    ncrcnode* node = NULL;
    ncrcnode* subnode = NULL;
    struct ncrclist list = {NULL,0,0};
    int token;

    ENTER(_map);
    node = createncrcnode(lexer,ncrc_map);
    for(;;) {
        token = nexttoken(lexer);
	if(optional && token == RBRACE)
	    FAIL(lexer,"brace mismatch");
	else if(optional && token == EOF) goto done;
	else if(!optional && token == EOF)
	    FAIL(lexer,"unclosed map");
	else if(!optional && token == RBRACE) goto done;
	if(token == COMMA || token == SEMICOLON) continue;
	pushtoken(lexer);
        if(!pair(lexer,&subnode)) goto fail;
	if(!listadd(&list,subnode)) goto fail;
    }
done:
    node->list.values = list.contents;
    node->list.nvalues = list.len;
    if(nodep) *nodep = node;
    LEAVE(_map,1);
    return 1;
fail:
    listclear(&list);
    if(subnode != NULL) ncrc_nodefree(subnode);
    if(node != NULL) ncrc_nodefree(node);
    LEAVE(_map,0);
    return 0;
}

static int
pair(ncrclexer* lexer, ncrcnode** nodep)
{
    ncrcnode* node = NULL;
    int token = 0;

    ENTER(_pair);
    node = createncrcnode(lexer,ncrc_pair);
    if(!node) goto fail;
    if(!makeconst(lexer,&node->pair.key))
	FAIL(lexer,"map key is not a string or word");
    if(node->pair.key->constclass != ncrc_string)
	FAIL(lexer,"map key is not a string or word");
    token = nexttoken(lexer);	
    if(token != COLON && token != EQUAL ) goto fail;
    if(!value(lexer,&node->pair.value))
	FAIL(lexer,"invalid map value");
    if(nodep) *nodep = node;
    LEAVE(_pair,1);
    return 1;
fail:
    if(node != NULL) ncrc_nodefree(node);
    LEAVE(_pair,0);
    return 0;
}

static int
array(ncrclexer* lexer, ncrcnode** nodep)
{
    ncrcnode* subnode = NULL;
    struct ncrclist list = {NULL,0,0};
    ncrcnode* node = NULL;

    ENTER(_array);

    node = createncrcnode(lexer,ncrc_array);
    for(;;) {
        int token = nexttoken(lexer);
	if(token == EOF)
	    FAIL(lexer,"unclosed array");
	if(token == RBRACK) goto done;
	if(token == COMMA || token == SEMICOLON) continue;
	pushtoken(lexer);
        if(!value(lexer,&subnode)) goto fail;
	if(!listadd(&list,subnode)) goto fail;
    }
done:
    node->list.values = list.contents;
    node->list.nvalues = list.len;
    if(nodep) *nodep = node;
    LEAVE(_array,1);
    return 1;
fail:
    listclear(&list);
    if(subnode != NULL) ncrc_nodefree(subnode);
    if(node != NULL) ncrc_nodefree(node);
    LEAVE(_array,0);
    return 0;
}

static int
makeconst(ncrclexer* lexer, ncrcnode** nodep)
{
    ncrcnode* node = NULL;
    int token;
    ENTER(_const);
    token = nexttoken(lexer);
    switch (token) {
    case _STRING:
    case _NUMBER:
	node = createncrcnode(lexer,ncrc_const);
	if(node == NULL) goto fail;
	node->constclass = token;
	node->constvalue = strdup(lexer->text.text);
	break;
    case _TRUE:
	node = &ncrc_constant_true;
	break;
    case _FALSE:
	node = &ncrc_constant_false;
	break;
    case _NULL:
	node = &ncrc_constant_null;
	break;
    default: abort();
    }        
    if(nodep) *nodep = node;
    LEAVE(_const,1);
    return 1;
fail:
    LEAVE(_const,0);
    return 0;
}

static ncrcnode*
createncrcnode(ncrclexer* lexer, ncrcclass cls)
{
     ncrcnode* node = (ncrcnode*)malloc(sizeof(ncrcnode));
     if(node != NULL) memset((void*)node,0,sizeof(ncrcnode));
     node->nodeclass = cls;
     return node;
}

/****************************************/

#ifdef IGNORE
static int
peek(ncrclexer* lexer)
{
    int token = nexttoken(lexer);
    pushtoken(lexer);
    return token;
}
#endif

static void
pushtoken(ncrclexer* lexer)
{
    lexer->pushedback = 1;
    if(ncrcdebug > 1)
	dumptoken(lexer,1);
}

static int
nexttoken(ncrclexer* lexer)
{
    int token;
    int c;
    if(lexer->pushedback)
	{token = lexer->token; lexer->pushedback = 0; goto done;}
    token = 0;
    textclear(&lexer->text);
    while(token==0) {
	c=readc(lexer);
	lexer->charno++;
	if(c == EOF) {
	    token = EOF;
	    lexer->charno--;
	    break;
	} else if(c == '\n') {
	    lexer->lineno++;
	    lexer->charno = 1;
	} else if(c == '/') { 
	    c = readc(lexer);
	    if(c == '/') {/* single line comment */
	        while((c=readc(lexer)) != EOF) {if(c == '\n') break;}
	    } else {
		pushback(lexer,c); c = '/';
	    }
	}
	if(c <= ' ' || c == '\177') {
	    /* ignore */
	} else if(strchr(delims,c) != NULL) {
	    textadd(&lexer->text,c);
	    token = c;
	} else if(c == '"') {
	    int more = 1;
	    while(more) {
		c = readc(lexer);
		switch (c) {
		case EOF: goto fail;
		case '"': more=0; break;
		case '\\':
		    textadd(&lexer->text,c);
		    c=readc(lexer);
		    textadd(&lexer->text,c);		    
		    break;
		default: textadd(&lexer->text,c);
		}
	    }
	    if(!removeescapes(lexer)) goto fail;
	    token=_STRING;
	} else { /* Treat like a string without quotes */
	    textadd(&lexer->text,c);
	    while((c=readc(lexer))) {
		if(c == '/' || c <= ' ' || c == '\177') {pushback(lexer,c); break;}
		else if(strchr(delims,c) != NULL) {pushback(lexer,c); break;}
		textadd(&lexer->text,c);
	    }
	    if(!removeescapes(lexer)) goto fail;
	    /* check for keyword */
	    if(strcmp(lexer->text.text,"true") == 0) {
		token = _TRUE;
	    } else if(strcmp(lexer->text.text,"false") == 0) {
		token = _FALSE;
	    } else if(strcmp(lexer->text.text,"null") == 0) {
		token = _NULL;
	    } else { /* See if this looks like a number */
		double d;
		if(sscanf(lexer->text.text,"%lg",&d) == 1)
		    token = _NUMBER;
		else
		    token = _STRING;
	    }
	}
    }
done:
    lexer->token = token;
    if(ncrcdebug > 1)
	dumptoken(lexer,0);
    return token;
fail:
    return EOF;
}

static unsigned int
tohex(int c)
{
    if(c >= 'a' && c <= 'f') return (c - 'a') + 0xa;
    if(c >= 'A' && c <= 'F') return (c - 'A') + 0xa;
    if(c >= '0' && c <= '9') return (c - '0');
    return 0;
}

static void
dumptoken(ncrclexer* lexer, int pushed)
{
    fprintf(stderr,"%s : %d = |%s|\n",
	(pushed?"PUSHED":"TOKEN"),
	lexer->token,lexer->text.text);
}


static void
textclear(struct ncrctext* txt)
{
    if(txt->len > 0) memset(txt->text,0,txt->alloc);
    txt->len = 0;
}

static int
textterminate(struct ncrctext* text)
{
    return textadd(text,'\0');
}

static int
textadd(struct ncrctext* text, int c)
{
    if(text->len >= text->alloc) {
        if(text->alloc == 0) {
	    text->text = (char*)malloc(ALLOCINCR+1);
	    if(text->text == NULL) return 0;
	    text->alloc = ALLOCINCR;
	    text->len = 0;
	} else {
	    text->text = (char*)realloc((void*)text->text,text->alloc+ALLOCINCR+1);
	    if(text->text == NULL) return 0;
	    text->alloc += ALLOCINCR;
	}
	text->text[text->alloc] = '\0';
    }
    text->text[text->len++] = c;
    return 1;    
}

static void
pushback(ncrclexer* lexer, int c)
{
    if(lexer->text.pushback[0] == 0) lexer->text.pushback[0] = c;
    else lexer->text.pushback[1] = c;
}

static int
unpush(ncrclexer* lexer)
{
    int c = '\0';
    int i;
    for(i=1;i>=0;i--) {
        if(lexer->text.pushback[i] != 0) {
	    c = lexer->text.pushback[i];
	    lexer->text.pushback[i] = '\0';
	    break;
	}
    }
    return c;
}

static int
readc(ncrclexer* lexer)
{
    int c = 0;

    c = unpush(lexer);
    if(c == 0) c = getc(lexer->input);
    return c;
}

/* Convert the characters in lexer->text.text to
   remove escapes. Assumes that all escapes are smaller
   than the unescaped value.
*/
static int
removeescapes(ncrclexer* lexer)
{
    char* p = lexer->text.text;
    char* q = p;
    int cp;
    while((cp=*p++)) {
        switch (cp) {
	case '\\':
	    cp=*p++;
	    switch (cp) {
	    case '\0': *q++ = cp; goto done;
            case 'r': *q++ = '\r'; break;
            case 'n': *q++ = '\n'; break;
            case 'f': *q++ = '\f'; break;
            case 't': *q++ = '\t'; break;
            case 'b': *q++ = '\b'; break;
            case '/': *q++ = '/'; break; /* ncrc requires */
            case 'x': {
                unsigned int d[2];
                int i;
                for(i=0;i<2;i++) {
                    if((cp = *p++) == '\0') goto fail;
                    d[i] = tohex(cp);
                }
                /* Convert to a sequence of utf-8 characters */
                cp = (d[0]<<4)|d[1];
		*q++ = cp;
            } break;

            default: break;

            }
            break;
        }
    }
done:
    return 1;
fail:
    return 0;
}


static int
listadd(ncrclist* list, ncrcnode* node)
{
    if(list->len >= list->alloc) {
        if(list->alloc == 0) {
	    list->contents = (ncrcnode**)malloc(sizeof(ncrcnode)*ALLOCINCR);
	    if(list->contents == NULL) return 0;
	    list->alloc = ALLOCINCR;
	    list->len = 0;
	} else {
	    list->contents = (ncrcnode**)realloc((void*)list->contents,sizeof(ncrcnode)*(list->alloc+ALLOCINCR));
	    if(list->contents == NULL) return 0;
	    list->alloc += ALLOCINCR;
	}
    }
    list->contents[list->len++] = node;
    return 1;    
}

static void
listclear(ncrclist* list)
{
    if(list->contents != NULL) free(list->contents);
}

void
ncrc_nodefree(ncrcnode* node)
{
    int i;
    if(node == NULL) return;
    switch (node->nodeclass) {
    case ncrc_map:
	for(i=0;i<node->list.nvalues;i++)
	    ncrc_nodefree(node->list.values[i]);
	break;
    case ncrc_array:
	for(i=0;i<node->list.nvalues;i++)
	    ncrc_nodefree(node->list.values[i]);
	break;

    case ncrc_pair:
	free(node->pair.key);
	if(node->pair.value != NULL)
	    ncrc_nodefree(node->pair.value);
	break;

    case ncrc_const:
        switch (node->constclass) {
        case ncrc_string:
        case ncrc_number:
	    if(node->constvalue) free(node->constvalue);
	    break;
        case ncrc_true:
        case ncrc_false:
        case ncrc_null:
	    break;
        default: abort();
	} break;
	break;

    default: abort();
    }
}

static void
indent(FILE* f, int depth)
{
#ifdef IGNORE
    while(depth--) fputs(INDENTCHUNK,f);
#endif
}

static void
stringify(char* s, struct ncrctext* tmp)
{
    char* p = s;
    int c;
    textclear(tmp);
    while((c=*p++)) {
	if(c == '"' || c < ' ' || c >= '\177') {
	    textadd(tmp,'\\');
	    switch (c) {
	    case '"': textadd(tmp,'"'); break;
	    case '\r': textadd(tmp,'r'); break;
	    case '\n': textadd(tmp,'r'); break;
	    case '\t': textadd(tmp,'r'); break;
	    default:
	        textadd(tmp,'x');
	        textadd(tmp,hexchars[(c & 0xf0)>>4]);
	        textadd(tmp,hexchars[c & 0x0f]);
		break;
	    }
	} else 
	    textadd(tmp,c);
    }
}

static int
isword(char* s)
{
    char* p = s;
    int c;
    while((c=*p++)) {
	if(strchr(delims,c) != NULL
	   || c == '/' || c <= ' ' || c >= '\177') return 0;
    }
    return 1;
}

static int
isoneline(ncrcnode* node)
{
    int i;
    for(i=0;i<node->list.nvalues;i++) {
	ncrcclass cl;
	ncrcnode* member;
	member = node->list.values[i];
	if(node->nodeclass == ncrc_map) {
    	    cl = member->pair.value->nodeclass;
	} else if(node->nodeclass == ncrc_array) {
    	    cl = member->nodeclass;
	} else return 0;
        if(cl == ncrc_array || cl == ncrc_map) return 0;
    }
    return 1;    
}

static void
ncrc_dumpr(ncrcnode* node, FILE* f, struct ncrctext* tmp, int depth, int meta)
{
    int i;
    int oneline;
    int endpoint;
    int lparen, rparen;
    char* tag = NULL;

    switch (node->nodeclass) {
    case ncrc_map:
	{lparen = LBRACE; rparen = RBRACE; tag = "<map>";}
	/* fall thru */
    case ncrc_array:
	if(tag == NULL)
	    {lparen = LBRACK; rparen = RBRACK; tag = "<array>";}
	oneline = isoneline(node);
	indent(f,depth);
	if(meta) fputs(tag,f);
	if(meta || depth > 0) fputc(lparen,f);
	endpoint = node->list.nvalues - 1;
	for(i=0;i<=endpoint;i++) {
	    ncrcnode* member = node->list.values[i];
	    if(i>0) fputs(" ",f);
	    ncrc_dumpr(member,f,tmp,depth+1,meta);
	    if(i<endpoint && !oneline) {fputs("\n",f); indent(f,depth);}
	}
	if(!oneline) {
	    if(i > 0) fputs("\n",f);
	    indent(f,depth);
	}
	if(meta || depth > 0) fputc(rparen,f);
	fputs("\n",f);
	break;

    case ncrc_pair:
        ncrc_dumpr(node->pair.key,f,tmp,depth+1,meta);
	fputs(" : ",f);
        ncrc_dumpr(node->pair.value,f,tmp,depth+1,meta);
	break;

    case ncrc_const:
        switch (node->constclass) {
        case ncrc_string:
	    if(meta) fputs("<string>",f);
	    stringify(node->constvalue,tmp);
	    textterminate(tmp);
	    if(isword(tmp->text))
	        fprintf(f,"%s",tmp->text);
	    else
		fprintf(f,"\"%s\"",tmp->text);
	    break;
        case ncrc_number:
	    if(meta) fputs("<number>",f);
	    fprintf(f,"%s",node->constvalue);
	    break;
        case ncrc_true:
	    if(meta) fputs("<true>",f);
	    fputs("true",f);
	    break;
        case ncrc_false:
	    if(meta) fputs("<false>",f);
	    fputs("false",f);
	    break;
        case ncrc_null:
	    if(meta) fputs("<null>",f);
	    fputs("null",f);
	    break;
        default: abort();
	}
	break;

    default: abort();
    }
}

void
ncrc_dump(ncrcnode* node, FILE* f)
{
    struct ncrctext tmp = {NULL,0,0};
    textclear(&tmp);
    ncrc_dumpr(node,f,&tmp,0,0);
}

void
ncrc_dumpmeta(ncrcnode* node, FILE* f)
{
    struct ncrctext tmp = {NULL,0,0};
    textclear(&tmp);
    ncrc_dumpr(node,f,&tmp,0,1);
}

static void
trace(enum nonterms nt, int leave, int ok)
{
    if(!leave) {
	fprintf(stderr,"enter: %s\n",nontermnames[(int)nt]);
    } else {/* leave */
	fprintf(stderr,"leave: %s : %s\n",nontermnames[(int)nt],
		(ok?"succeed":"fail"));
    }
}

/**************************************************/

ncrcnode*
ncrc_lookup(ncrcnode* node, char* key)
{
    int i;
    if(node->nodeclass != ncrc_map) return NULL;
    for(i=0;i<node->list.nvalues;i++) {
	ncrcnode* pair = node->list.values[i];
	if(strcmp(pair->pair.key->constvalue,key)==0)
	    return pair->pair.value;
    }
    return NULL;
}

ncrcnode*
ncrc_get(ncrcnode* node, int index)
{
    int i;
    if(node->nodeclass == ncrc_map || node->nodeclass == ncrc_array) {
        if(index < 0 || index >= node->list.nvalues) return NULL;
	return node->list.values[i];	
    }
    return NULL;
}


/**************************************************/
/* Provide support for url matching */

/* match = true  if pattern is a prefix of the url */
static int
ncrc_match(char* pattern, char* url)
{
    if(strncmp(url,pattern,strlen(pattern))==0) return 1;
    return 0;
}

/* Move list elements pos .. len-1 up 1 */
static void
ncrc_shiftup(ncrcnode** list, int* lenp, int pos)
{
    int i;
    int len = *lenp;
    for(i=(len-1);i>=pos;i--) list[i+1] = list[i]; /* shift up */
    *lenp = len;
}

/* Sort the list of matches in increasing length
   so that more specific controls cant prevail
*/
static void
ncrc_insert(ncrcnode** matches, int* lenp, ncrcnode* pair)
{
    int i,j;
    int len = *lenp;
    /* handle initial case separately */
    if(len > 0) {
        /* sort lexically as determined by strcmp */
        for(i=0;i<len;i++) {
	    ncrcnode* node = matches[i];
	    if(node->nodeclass != ncrc_pair) continue; /* ignore */
	    if(strlen(node->pair.key->constvalue) > strlen(pair->pair.key->constvalue)) {
		ncrc_shiftup(matches,&j,i);
	        matches[i] = pair;
		goto done;
	    }
	}
    }
    matches[len] = pair; /* default: add at end */
done:
    len++;
    *lenp = len;
}

/* Replace all the pairs in the match set to their values */
/* the values should all be maps */
static void
ncrc_depair(ncrcnode** matches, int nmatches)
{
    int i;
    for(i=0;i<nmatches;i++) {
	ncrcnode* node = matches[i];
	if(node->nodeclass == ncrc_pair)
    	    matches[i] = node->pair.value;
	assert(matches[i]->nodeclass == ncrc_map);
    }
}

static int
ncrc_collectsortedmatches(char* url, ncrcnode* map, ncrcnode*** matchesp)
{
    int i,count;
    int nvalues = map->list.nvalues;
    ncrcnode* starvalue;
    ncrcnode** matches = NULL;
    if(map == NULL || map->nodeclass != ncrc_map) {count=0; goto done;}
    matches = (ncrcnode**)malloc(sizeof(ncrcnode*)*(nvalues+2));
    assert(map->nodeclass == ncrc_map);

    /* start by inserting this map and then the "*" entry (if any) */

    count = 0;
    /* Add whole map at front */
    matches[count++] = map;

    /* Add "*" key at front */
    starvalue = ncrc_lookup(map,"*");
    if(starvalue && starvalue->nodeclass == ncrc_map) {
	matches[count++] = starvalue;
    }

    for(i=0;i<nvalues;i++) {
	ncrcnode* pair = map->list.values[i];
	if(pair->nodeclass != ncrc_pair
	   && pair->pair.value->nodeclass != ncrc_map) continue;
	if(strcmp(pair->pair.key->constvalue,"*")==0) continue;
	if(ncrc_match(pair->pair.key->constvalue,url)) {
	    ncrc_insert(matches,&count,pair);
	}
    }
    /* convert the pairs */
    ncrc_depair(matches,count);

done:
    if(matchesp) *matchesp = matches;
    return count;
}

int
ncrc_urlmatch(ncrcnode* map, char* url, ncrcnode*** matchp)
{
    int matchcount = 0;
    ncrcnode** matches = NULL;
    assert(map->nodeclass == ncrc_map);
    matchcount = ncrc_collectsortedmatches(url,map,&matches);
    if(matchp) *matchp = matches;
    return matchcount;
}
