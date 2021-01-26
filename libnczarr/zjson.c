/* Copyright 2018, UCAR/Unidata.
   See the COPYRIGHT file for more information.
*/

#include "zincludes.h"
#include "zjson.h"

#undef DEBUG

#define NCJ_LBRACKET '['
#define NCJ_RBRACKET ']'
#define NCJ_LBRACE '{'
#define NCJ_RBRACE '}'
#define NCJ_COLON ':'
#define NCJ_COMMA ','
#define NCJ_QUOTE '"'
#define NCJ_ESCAPE '\\'
#define NCJ_TAG_TRUE "true"
#define NCJ_TAG_FALSE "false"
#define NCJ_TAG_NULL "null"

#define NCJ_EOF -1
#define NCJ_ERR -2

/* WORD Subsumes Number also */
#define WORD "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_$+-."

/*//////////////////////////////////////////////////*/

typedef struct NCJparser {
    char* text;
    char* pos;
    size_t yylen; /* |yytext| */
    char* yytext; /* string or word */
    long long num;
    int tf;
    int err;
} NCJparser;

/**************************************************/
/* Forward */
static int NCJparseR(NCJparser* parser, NCjson**);
static int NCJparseArray(NCJparser* parser, NClist* array);
static int NCJparseDict(NCJparser* parser, NClist* dict);
static int testbool(const char* word);
static int testint(const char* word);
static int testdouble(const char* word);
static int testnull(const char* word);
static int NCJlex(NCJparser* parser);
static int NCJyytext(NCJparser*, char* start, ptrdiff_t pdlen);
static void NCJreclaimArray(NClist*);
static void NCJreclaimDict(NClist*);
static int NCJunparseR(const NCjson* json, NCbytes* buf, int flags);
static int NCJunescape(NCJparser* parser);
static int NCJescape(const char* text, NCbytes* buf);
static int NCJappendquoted(const char* value, NCbytes* buf);
static int NCJcloneArray(NClist* array, NCjson** clonep);
static int NCJcloneDict(NClist* dict, NCjson** clonep);

#ifdef DEBUG
static char* tokenname(int token);
#endif
/**************************************************/
int
NCJparse(const char* text, unsigned flags, NCjson** jsonp)
{
    int stat = NC_NOERR;
    size_t len;
    NCJparser* parser = NULL;
    NCjson* json = NULL;

    /* Need at least 1 character of input */
    if(text == NULL || text[0] == '\0')
	{stat = NC_EINVAL; goto done;}
    if(jsonp == NULL) goto done;
    parser = calloc(1,sizeof(NCJparser));
    if(parser == NULL)
	{stat = NC_ENOMEM; goto done;}
    len = strlen(text);
    parser->text = (char*)malloc(len+1+1);
    if(parser->text == NULL)
	{stat = NC_ENOMEM; goto done;}
    parser->text[0] = '\0';
    strlcat(parser->text,text,len+1);
    parser->text[len] = '\0';
    parser->text[len+1] = '\0';
    parser->pos = &parser->text[0];
#ifdef DEBUG
fprintf(stderr,"json: |%s|\n",parser->text);
#endif
    if((stat=NCJparseR(parser,&json))) goto done;
    *jsonp = json;
    json = NULL;

done:
    if(parser != NULL) {
	nullfree(parser->text);
	nullfree(parser->yytext);
	free(parser);
    }
    (void)NCJreclaim(json);
    return THROW(stat);
}

/*
Simple recursive descent
intertwined with dict and list parsers.

Invariants:
1. The json argument is provided by caller and filled in by NCJparseR.
2. Each call pushed back last unconsumed token
*/

static int
NCJparseR(NCJparser* parser, NCjson** jsonp)
{
    int stat = NC_NOERR;
    int token = NCJ_ERR;
    NCjson* json = NULL;

    if(jsonp == NULL)
	{stat = NC_EINTERNAL; goto done;}
    if((token = NCJlex(parser)) == NCJ_ERR)
	{stat = NC_EINVAL; goto done;}
    switch (token) {
    case NCJ_EOF:
	break;
    case NCJ_NULL:
        if((stat = NCJnew(NCJ_NULL,&json))) goto done;
	break;
    case NCJ_BOOLEAN:
        if((stat = NCJnew(NCJ_BOOLEAN,&json))) goto done;
	json->value = strdup(parser->yytext);
	break;
    case NCJ_INT:
        if((stat = NCJnew(NCJ_INT,&json))) goto done;
	json->value = strdup(parser->yytext);
	break;
    case NCJ_DOUBLE:
        if((stat = NCJnew(NCJ_DOUBLE,&json))) goto done;
	json->value = strdup(parser->yytext);
	break;
    case NCJ_STRING:
        if((stat = NCJnew(NCJ_STRING,&json))) goto done;
	json->value = strdup(parser->yytext);
	break;
    case NCJ_LBRACE:
        if((stat = NCJnew(NCJ_DICT,&json))) goto done;
	if((stat = NCJparseDict(parser, json->contents))) goto done;
	break;
    case NCJ_LBRACKET:
        if((stat = NCJnew(NCJ_ARRAY,&json))) goto done;
	if((stat = NCJparseArray(parser, json->contents))) goto done;
	break;
    case NCJ_RBRACE: /* We hit end of the dict we are parsing */
	parser->pos--; /* pushback so NCJparseArray will catch */
	json = NULL;
	break;
    case NCJ_RBRACKET:
	parser->pos--; /* pushback so NCJparseDict will catch */
	json = NULL;
	break;
    default:
	stat = NC_EINVAL;
	break;
    }
    if(jsonp && json) {*jsonp = json; json = NULL;}

done:
    NCJreclaim(json);
    return THROW(stat);
}

static int
NCJparseArray(NCJparser* parser, NClist* array)
{
    int stat = NC_NOERR;
    int token = NCJ_ERR;
    NCjson* element = NULL;
    int stop = 0;

    /* [ ^e1,e2, ...en] */

    while(!stop) {
	/* Recurse to get the value ei (might be null) */
	if((stat = NCJparseR(parser,&element))) goto done;
	token = NCJlex(parser); /* Get next token */
	/* Next token should be comma or rbracket */
	switch(token) {
	case NCJ_RBRACKET:
	    if(element != NULL) nclistpush(array,element);
	    element = NULL;
	    stop = 1;
	    break;
	case NCJ_COMMA:
	    /* Append the ei to the list */
	    if(element == NULL) {stat = NC_EINVAL; goto done;} /* error */
	    nclistpush(array,element);
	    element = NULL;
	    break;
	case NCJ_EOF:
	case NCJ_ERR:
	default:
	    stat = NC_EINVAL;
	    goto done;
	}	
    }	

done:
    if(element != NULL)
	NCJreclaim(element);
    return THROW(stat);
}

static int
NCJparseDict(NCJparser* parser, NClist* dict)
{
    int stat = NC_NOERR;
    int token = NCJ_ERR;
    NCjson* value = NULL;
    NCjson* key = NULL;
    int stop = 0;

    /* { ^k1:v1,k2:v2, ...kn:vn] */

    while(!stop) {
	/* Get the key, which must be a word of some sort */
	token = NCJlex(parser);
	switch(token) {
	case NCJ_STRING:
	case NCJ_BOOLEAN:
	case NCJ_INT: case NCJ_DOUBLE: {
 	    if((stat=NCJnewstring(token,parser->yytext,&key))) goto done;
	    } break;
	case NCJ_RBRACE: /* End of containing Dict */
	    stop = 1;
	    continue; /* leave loop */
	case NCJ_EOF: case NCJ_ERR:
	default:
	    stat = NC_EINVAL;
	    goto done;
	}
	/* Next token must be colon*/
   	switch((token = NCJlex(parser))) {
	case NCJ_COLON: break;
	case NCJ_ERR: case NCJ_EOF:
	default: stat = NC_EINVAL; goto done;
	}    
	/* Get the value */
	if((stat = NCJparseR(parser,&value))) goto done;
        /* Next token must be comma or RBRACE */
	switch((token = NCJlex(parser))) {
	case NCJ_RBRACE:
	    stop = 1;
	    /* fall thru */
	case NCJ_COMMA:
	    /* Insert key value into dict: key first, then value */
	    nclistpush(dict,key);
	    key = NULL;
	    nclistpush(dict,value);
	    value = NULL;
	    break;
	case NCJ_EOF:
	case NCJ_ERR:
	default:
	    stat = NC_EINVAL;
	    goto done;
	}	
    }	

done:
    if(key != NULL)
	NCJreclaim(key);
    if(value != NULL)
	NCJreclaim(value);
    return THROW(stat);
}

static int
NCJlex(NCJparser* parser)
{
    int c;
    int token = 0;
    char* start;
    ptrdiff_t count;

    while(token == 0) { /* avoid need to goto when retrying */
	c = *parser->pos;
	if(c == '\0') {
	    token = NCJ_EOF;
	} else if(c <= ' ' || c == '\177') {
	    parser->pos++;
	    continue; /* ignore whitespace */
	} else if(strchr(WORD, c) != NULL) {
	    start = parser->pos;
	    for(;;) {
		c = *parser->pos++;
		if(strchr(WORD,c) == NULL) break; /* end of word */
	    }
	    /* Pushback c if not whitespace */
	    if(c > ' ' && c != '\177') parser->pos--;
	    count = ((parser->pos) - start);
	    if(NCJyytext(parser,start,count)) goto done;
	    /* Discriminate the word string to get the proper sort */
	    if(testbool(parser->yytext) == NC_NOERR)
		token = NCJ_BOOLEAN;
	    /* do int test first since double subsumes int */
	    else if(testint(parser->yytext) == NC_NOERR)
		token = NCJ_INT;
	    else if(testdouble(parser->yytext) == NC_NOERR)
		token = NCJ_DOUBLE;
	    else if(testnull(parser->yytext) == NC_NOERR)
		token = NCJ_NULL;
	    else
		token = NCJ_STRING;
	} else if(c == NCJ_QUOTE) {
	    parser->pos++;
	    start = parser->pos;
	    for(;;) {
		c = *parser->pos++;
		if(c == NCJ_ESCAPE) c++;
		else if(c == NCJ_QUOTE || c == '\0') break;
	    }
	    if(c == '\0') {
		parser->err = NC_EINVAL;
		token = NCJ_ERR;
		goto done;
	    }
	    count = ((parser->pos) - start) - 1; /* -1 for trailing quote */
	    if(NCJyytext(parser,start,count)) goto done;
	    if(NCJunescape(parser)) goto done;
	    token = NCJ_STRING;
	} else { /* single char token */
	    if(NCJyytext(parser,parser->pos,1)) goto done;
	    token = *parser->pos++;
	}
#ifdef DEBUG
fprintf(stderr,"%s(%d): |%s|\n",tokenname(token),token,parser->yytext);
#endif
    } /*for(;;)*/
done:
    if(parser->err) token = NCJ_ERR;
    return token;
}

static int
testnull(const char* word)
{
    if(strcasecmp(word,NCJ_TAG_NULL)==0)
	return NC_NOERR;
    return NC_EINVAL;
}

static int
testbool(const char* word)
{
    if(strcasecmp(word,NCJ_TAG_TRUE)==0
       || strcasecmp(word,NCJ_TAG_FALSE)==0)
	return NC_NOERR;
    return NC_EINVAL;
}

static int
testint(const char* word)
{
    int ncvt;
    long long i;
    int count = 0;
    /* Try to convert to number */
    ncvt = sscanf(word,"%lld%n",&i,&count);
    return (ncvt == 1 && strlen(word)==count ? NC_NOERR : NC_EINVAL);
}

static int
testdouble(const char* word)
{
    int ncvt;
    double d;
    int count = 0;
    /* Check for Nan and Infinity */
    if(strcasecmp("nan",word)==0) return NC_NOERR;
    if(strcasecmp("infinity",word)==0) return NC_NOERR;
    if(strcasecmp("-infinity",word)==0) return NC_NOERR;
    /* Allow the XXXf versions as well */
    if(strcasecmp("nanf",word)==0) return NC_NOERR;
    if(strcasecmp("infinityf",word)==0) return NC_NOERR;
    if(strcasecmp("-infinityf",word)==0) return NC_NOERR;
    /* Try to convert to number */
    ncvt = sscanf(word,"%lg%n",&d,&count);
    return (ncvt == 1 && strlen(word)==count ? NC_NOERR : NC_EINVAL);
}

static int
NCJyytext(NCJparser* parser, char* start, ptrdiff_t pdlen)
{
    size_t len = (size_t)pdlen;
    if(parser->yytext == NULL) {
	parser->yytext = (char*)malloc(len+1);
	parser->yylen = len;
    } else if(parser->yylen <= len) {
	parser->yytext = (char*) realloc(parser->yytext,len+1);
	parser->yylen = len;
    }
    if(parser->yytext == NULL) return NC_ENOMEM;
    memcpy(parser->yytext,start,len);
    parser->yytext[len] = '\0';
    return NC_NOERR;
}

/**************************************************/

void
NCJreclaim(NCjson* json)
{
    if(json == NULL) return;
    switch(json->sort) {
    case NCJ_INT:
    case NCJ_DOUBLE:
    case NCJ_BOOLEAN:
    case NCJ_STRING: 
	nullfree(json->value);
	break;
    case NCJ_DICT:
	NCJreclaimDict(json->contents);
	nclistfree(json->contents);
	break;
    case NCJ_ARRAY:
	NCJreclaimArray(json->contents);
	nclistfree(json->contents);
	break;
    default: break; /* nothing to reclaim */
    }
    free(json);
}

static void
NCJreclaimArray(NClist* array)
{
    int i;
    for(i=0;i<nclistlength(array);i++) {
	NCjson* j = nclistget(array,i);
	NCJreclaim(j);
    }
}

static void
NCJreclaimDict(NClist* dict)
{
    return NCJreclaimArray(dict);
}

int
NCJclone(NCjson* json, NCjson** clonep)
{
    int stat = NC_NOERR;
    NCjson* clone = NULL;
    if(json == NULL) goto done;
    switch(json->sort) {
    case NCJ_INT:
    case NCJ_DOUBLE:
    case NCJ_BOOLEAN:
    case NCJ_STRING:
	if((stat=NCJnew(json->sort,&clone))) goto done;
	if((clone->value = strdup(json->value)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	break;
    case NCJ_NULL:
	if((stat=NCJnew(json->sort,&clone))) goto done;
	break;
    case NCJ_DICT:
	if((stat=NCJcloneDict(json->contents,&clone))) goto done;
	break;
    case NCJ_ARRAY:
	if((stat=NCJcloneArray(json->contents,&clone))) goto done;
	break;
    default: break; /* nothing to clone */
    }
done:
    if(stat == NC_NOERR && clonep) {*clonep = clone; clone = NULL;}
    NCJreclaim(clone);    
    return stat;
}

static int
NCJcloneArray(NClist* array, NCjson** clonep)
{
    int i, stat=NC_NOERR;
    NCjson* clone = NULL;
    if((stat=NCJnew(NCJ_ARRAY,&clone))) goto done;
    for(i=0;i<nclistlength(array);i++) {
	NCjson* elem = nclistget(array,i);
	NCjson* elemclone = NULL;
	if((stat=NCJclone(elem,&elemclone))) goto done;
	nclistpush(clone->contents,elemclone);
    }
done:
    if(stat == NC_NOERR && clonep) {*clonep = clone; clone = NULL;}
    NCJreclaim(clone);    
    return stat;
}

static int
NCJcloneDict(NClist* dict, NCjson** clonep)
{
    int stat = NC_NOERR;
    NCjson* clone = NULL;
    if((stat=NCJcloneArray(dict,&clone))) goto done;
    /* Convert from array to dict */
    clone->sort = NCJ_DICT;
done:
    if(stat == NC_NOERR && clonep) {*clonep = clone; clone = NULL;}
    NCJreclaim(clone);    
    return stat;
}

/**************************************************/
/* Build Functions */

int
NCJnew(int sort, NCjson** objectp)
{
    int stat = NC_NOERR;
    NCjson* object = NULL;

    if((object = calloc(1,sizeof(NCjson))) == NULL)
	{stat = NC_ENOMEM; goto done;}
    object->sort = sort;
    switch (sort) {
    case NCJ_INT:
    case NCJ_DOUBLE:
    case NCJ_BOOLEAN:
    case NCJ_STRING:
    case NCJ_NULL:
	break;
    case NCJ_DICT:
	object->contents = nclistnew();
	break;
    case NCJ_ARRAY:
	object->contents = nclistnew();
	break;
    default: 
	stat = NC_EINVAL;
	goto done;
    }
    if(objectp) {*objectp = object; object = NULL;}

done:
    if(stat) NCJreclaim(object);
    return THROW(stat);
}

int
NCJnewstring(int sort, const char* value, NCjson** jsonp)
{
    return NCJnewstringn(sort,strlen(value),value,jsonp);
}

int
NCJnewstringn(int sort, size_t len, const char* value, NCjson** jsonp)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    if(jsonp) *jsonp = NULL;
    if((stat = NCJnew(sort,&json)))
	goto done;
    if((json->value = malloc(len+1))==NULL)
        {stat = NC_ENOMEM; goto done;}
    memcpy(json->value,value,len);
    json->value[len] = '\0';
    if(jsonp) *jsonp = json;
    json = NULL; /* avoid memory errors */
done:
    if(stat) NCJreclaim(json);
    return THROW(stat);
}

/* Insert key-value pair into a dict object.
   key will be strdup'd
*/
int
NCJinsert(NCjson* object, char* key, NCjson* value)
{
    if(object == NULL || object->sort != NCJ_DICT)
	return NC_EINTERNAL;
    NCJaddstring(object,NCJ_STRING,key);
    nclistpush(object->contents,value);
    return NC_NOERR;
}

int
NCJaddstring(NCjson* dictarray, int sort, const char* value)
{
    int stat = NC_NOERR;
    NCjson* jvalue = NULL;
    switch (dictarray->sort) {
    case NCJ_DICT:
	if((stat = NCJnewstring(sort,value,&jvalue))) goto done;
	nclistpush(dictarray->contents,jvalue);
	break;
    case NCJ_ARRAY:
	if((stat = NCJnewstring(sort,value,&jvalue))) goto done;
	nclistpush(dictarray->contents,jvalue);
	break;
    default: stat = NC_EINVAL; goto done;
    }

done:
    return THROW(stat);
}

int
NCJdictith(NCjson* object, size_t i, const char** keyp, NCjson** valuep)
{
    if(object == NULL || object->sort != NCJ_DICT)
	return NC_EINTERNAL;
    if(i >= (nclistlength(object->contents)/2))
	return NC_EINVAL;
    if(keyp) *keyp = nclistget(object->contents,2*i);
    if(valuep) *valuep = nclistget(object->contents,(2*i)+1);
    return NC_NOERR;
}

int
NCJdictget(NCjson* object, const char* key, NCjson** valuep)
{
    int i;
    if(object == NULL || object->sort != NCJ_DICT)
	return NC_EINTERNAL;
    if(valuep) *valuep = NULL;
    for(i=0;i<nclistlength(object->contents);i+=2) {
	const NCjson* k = nclistget(object->contents,i);
	assert(k != NULL && k->sort == NCJ_STRING);
	if(strcmp(k->value,key)==0) {
            if(valuep) *valuep = nclistget(object->contents,i+1);
	    break;
	}
    }
    return NC_NOERR;
}

/* Append value to an array or dict object.
*/
int
NCJappend(NCjson* object, NCjson* value)
{
    if(object == NULL)
	return NC_EINTERNAL;
    switch (object->sort) {
    case NCJ_ARRAY:
        nclistpush(object->contents,value);
	break;
    case NCJ_DICT:
        nclistpush(object->contents,value);
	break;
    default:
	return NC_EINTERNAL;
    }
    return NC_NOERR;
}

int
NCJarrayith(NCjson* object, size_t i, NCjson** valuep)
{
    if(object == NULL || object->sort != NCJ_DICT)
	return NC_EINTERNAL;
    if(valuep) *valuep = nclistget(object->contents,i);
    return NC_NOERR;
}

/* Unescape the text in parser->yytext; can
   do in place because unescaped string will
   always be shorter */
static int
NCJunescape(NCJparser* parser)
{
    char* p = parser->yytext;
    char* q = p;
    int c;
    for(;(c=*p++);) {
	if(c == NCJ_ESCAPE) {
	    c = *p++;
	    switch (c) {
	    case 'b': c = '\b'; break;
	    case 'f': c = '\f'; break;
	    case 'n': c = '\n'; break;
	    case 'r': c = '\r'; break;
	    case 't': c = '\t'; break;
	    case NCJ_QUOTE: c = c; break;
	    case NCJ_ESCAPE: c = c; break;
	    default: c = c; break;/* technically not Json conformant */
	    }
	}
	*q++ = c;
    }
    *q = '\0';
    return NC_NOERR;    
}

/**************************************************/
/* Unparser to convert NCjson object to text in buffer */

int
NCJunparse(const NCjson* json, int flags, char** textp)
{
    int stat = NC_NOERR;
    NCbytes* buf = ncbytesnew();
    if((stat = NCJunparseR(json,buf,flags)))
	goto done;
    if(textp) {
	ncbytesnull(buf);
	*textp = ncbytesextract(buf);
    }
done:
    ncbytesfree(buf);
    return THROW(stat);
}

static int
NCJunparseR(const NCjson* json, NCbytes* buf, int flags)
{
    int stat = NC_NOERR;
    int i;
    switch (json->sort) {
    case NCJ_STRING:
	NCJappendquoted(json->value,buf);
	break;
    case NCJ_INT:
    case NCJ_DOUBLE:
    case NCJ_BOOLEAN:
	ncbytescat(buf,json->value);
	break;
    case NCJ_DICT:
	ncbytesappend(buf,NCJ_LBRACE);
	for(i=0;i<nclistlength(json->contents);) {
	    const NCjson* key = NULL;
	    const NCjson* value = NULL;
	    if(i > 0) ncbytesappend(buf,NCJ_COMMA);
	    key = nclistget(json->contents,i);
	    NCJunparseR(key,buf,flags);
	    ncbytesappend(buf,NCJ_COLON);
	    ncbytesappend(buf,' ');
	    /* Allow for the possibility of a short dict entry */
	    i++;
	    if(i >= nclistlength(json->contents)) { /*short*/
		ncbytescat(buf,"?");		
	    } else {
	        value = nclistget(json->contents,i);
	        NCJunparseR(value,buf,flags);
		i++;
	    }
	}	
	ncbytesappend(buf,NCJ_RBRACE);
	break;
    case NCJ_ARRAY:
	ncbytesappend(buf,NCJ_LBRACKET);
	for(i=0;i<nclistlength(json->contents);i++) {
	    const NCjson* value = nclistget(json->contents,i);
	    if(i > 0) ncbytesappend(buf,NCJ_COMMA);
	    NCJunparseR(value,buf,flags);
	}	
	ncbytesappend(buf,NCJ_RBRACKET);
	break;
    case NCJ_NULL:
	ncbytescat(buf,"null");
	break;
    default:
	stat = NC_EINVAL; goto done;
    }
done:
    return THROW(stat);
}

/* Escape a string and append to buf */
static int
NCJescape(const char* text, NCbytes* buf)
{
    const char* p = text;
    int c;
    for(;(c=*p++);) {
        char replace = 0;
        switch (c) {
	case '\b': replace = 'b'; break;
	case '\f': replace = 'f'; break;
	case '\n': replace = 'n'; break;
	case '\r': replace = 'r'; break;
	case '\t': replace = 't'; break;
	case NCJ_QUOTE: replace = '\''; break;
	case NCJ_ESCAPE: replace = '\\'; break;
	default: break;
	}
	if(replace) {
	    ncbytesappend(buf,NCJ_ESCAPE);
	    ncbytesappend(buf,replace);
	} else
	    ncbytesappend(buf,c);
    }
    return NC_NOERR;    
}

static int
NCJappendquoted(const char* value, NCbytes* buf)
{
    ncbytesappend(buf,'"');
    NCJescape(value,buf);
    ncbytesappend(buf,'"');
    return NC_NOERR;
}

void
NCJdump(const NCjson* json, int flags)
{
    char* text = NULL;
    (void)NCJunparse(json,0,&text);
    fprintf(stderr,"%s\n",text);
    fflush(stderr);
}

#ifdef DEBUG
static char*
tokenname(int token)
{
    switch (token) {
    case NCJ_STRING: return "NCJ_STRING";
    case NCJ_INT: return "NCJ_INT";
    case NCJ_DOUBLE: return "NCJ_DOUBLE";
    case NCJ_BOOLEAN: return "NCJ_BOOLEAN";
    case NCJ_DICT: return "NCJ_DICT";
    case NCJ_ARRAY: return "NCJ_ARRAY";
    case NCJ_NULL: return "NCJ_NULL";
    default:
	if(token > ' ' && token <= 127) {
	    static char s[4];
	    s[0] = '\'';
	    s[1] = (char)token;
	    s[2] = '\'';
	    s[3] = '\0';
	    return s;
	} else
	    break;
    }
    return "NCJ_UNDEF";
}
#endif
