/* Copyright 2018, UCAR/Unidata
   See the COPYRIGHT file for more information. */

#ifndef NCVUTILS_H
#define NCVUTILS_H 1

/* Define a header-only simple version of a dynamically expandable list and byte buffer */
/* To be used in code that should be independent of libnetcdf */

typedef struct VList {
  size_t alloc;
  size_t length;
  void** content;
} VList;

typedef struct VString {
  int nonextendible; /* 1 => fail if an attempt is made to extend this string*/
  size_t alloc;
  size_t length;
  char* content;
} VString;

/* VString has a fixed expansion size */
#define VSTRALLOC 64

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

static int util_initialized = 0;

static void util_initialize(void);

/**************************************************/
/* Forward */
static VList* vlistnew(void);
static void vlistfree(VList* l);
static void vlistexpand(VList* l);
static void* vlistget(VList* l, size_t index) /* Return the ith element of l */;
static void vlistpush(VList* l, void* elem);
static void* vlistfirst(VList* l) /* remove first element */;
static void vlistinsert(VList* l, size_t pos, void* elem);
static void* vlistremove(VList* l, size_t pos);
static void vlistfreeall(VList* l) /* call free() on each list element*/;
static VString* vsnew(void);
static void vsfree(VString* vs);
static void vsexpand(VString* vs);
static void vsappendn(VString* vs, const char* elem, size_t n);
static void vsappend(VString* vs, const char elem);
static void vsinsertn(VString* vs, size_t pos, const void* s, size_t slen);
static void vsremoven(VString* vs, size_t pos, size_t elide);
static void vssetalloc(VString* vs, size_t newalloc);
static void vssetlength(VString* vs, size_t newlen);
static void vssetcontents(VString* vs, char* contents, size_t length);
static char* vsextract(VString* vs);
static char* vsgetp(VString* vs, size_t index);
static VString* vsclone(VString* vs);

/* Following are always "in-lined"*/
#define vlistcontents(l)  ((l)==NULL?NULL:(l)->content)
#define vlistlength(l)  ((l)==NULL?0:(l)->length)
#define vlistclear(l)  vlistsetlength(l,0)
#define vlistsetlength(l,len)  do{if((l)!=NULL) (l)->length=len;} while(0)

#define vscontents(vs)  ((vs)==NULL?NULL:(vs)->content)
#define vslength(vs)  ((vs)==NULL?0:(vs)->length)
#define vscat(vs,s)  vsappendn(vs,s,0)
#define vsclear(vs)  vssetlength(vs,0)

#define vsnulterm(vs) do{vssetalloc(vs,vs->length+1); vs->content[vs->length] = '\0';}while(0)

/**************************************************/

static VList*
vlistnew(void)
{
  VList* l;
  if(!util_initialized) util_initialize();
  l = (VList*)calloc(1,sizeof(VList));
  assert(l != NULL);
  return l;
}

static void
vlistfree(VList* l)
{
  if(l == NULL) return;
  if(l->content != NULL) {free(l->content); l->content = NULL;}
  free(l);
}

static void
vlistexpand(VList* l)
{
  void** newcontent = NULL;
  size_t newsz;

  if(l == NULL) return;
  newsz = (l->length * 2) + 1; /* basically double allocated space */
  if(l->alloc >= newsz) return; /* space already allocated */
  newcontent=(void**)calloc(newsz,sizeof(void*));
  assert(newcontent != NULL);
  if(l->alloc > 0 && l->length > 0 && l->content != NULL) { /* something to copy */
    memcpy((void*)newcontent,(void*)l->content,sizeof(void*)*l->length);
  }
  if(l->content != NULL) free(l->content);
  l->content=newcontent;
  l->alloc=newsz;
  /* size is the same */  
}

static void*
vlistget(VList* l, size_t index) /* Return the ith element of l */
{
  if(l == NULL || l->length == 0) return NULL;
  assert(index < l->length);
  return l->content[index];
}

static void
vlistpush(VList* l, void* elem)
{
  if(l == NULL) return;
  while(l->alloc < (l->length+1)) vlistexpand(l);
  l->content[l->length] = elem;
  l->length++;
}

static void*
vlistfirst(VList* l) /* remove first element */
{
  size_t i,len;
  void* elem;
  if(l == NULL || l->length == 0) return NULL;
  elem = l->content[0];
  len = l->length;
  for(i=1;i<len;i++) l->content[i-1] = l->content[i];
  l->length--;
  return elem;  
}

static void
vlistinsert(VList* l, size_t pos, void* elem)
{
  size_t delta;
  assert(l != NULL);
  if(pos >= vlistlength(l)) return;
  while(l->alloc < (l->length+1)) vlistexpand(l);
  assert(l->length == pos);
  delta = (l->length - pos);
  memmove(l->content+pos+1,l->content+pos,sizeof(void*)*delta);
  l->length++;
  l->content[pos] = elem;
}

static void*
vlistremove(VList* l, size_t pos)
{
  size_t delta;
  void* old = NULL;
  assert(l != NULL && l->length > 0);
  assert(pos < vlistlength(l));
  old = l->content[pos];
  assert(l->length > pos);
  delta = (l->length - pos) - 1;
  memmove(l->content+pos,l->content+pos+1,sizeof(void*)*delta);
  l->length++;
  return old;
}

static void
vlistfreeall(VList* l) /* call free() on each list element*/
{
  unsigned i;
  if(l == NULL || l->length == 0) return;
  for(i=0;i<l->length;i++) if(l->content[i] != NULL) {free(l->content[i]);}
  vlistfree(l);
}

static VString*
vsnew(void)
{
  VString* vs = NULL;
  if(!util_initialized) util_initialize();
  vs = (VString*)calloc(1,sizeof(VString));
  assert(vs != NULL);
  return vs;
}

static void
vsfree(VString* vs)
{
  if(vs == NULL) return;
  if(vs->content != NULL) free(vs->content);
  free(vs);
}

static void
vsexpand(VString* vs)
{
  char* newcontent = NULL;
  size_t newsz;

  if(vs == NULL) return;
  assert(vs->nonextendible == 0);
  newsz = (vs->alloc + VSTRALLOC); /* increasse allocated space by fixed amount */
  if(vs->alloc >= newsz) return; /* space already allocated */
  newcontent=(char*)calloc(1,newsz+1);/* always room for nul term */
  assert(newcontent != NULL);
  if(vs->alloc > 0 && vs->length > 0 && vs->content != NULL) /* something to copy */
    memcpy((void*)newcontent,(void*)vs->content,vs->length);
  newcontent[vs->length] = '\0'; /* ensure null terminated */
  if(vs->content != NULL) free(vs->content);
  vs->content=newcontent;
  vs->alloc=newsz;
  /* length is the same */  
}

static void
vsappendn(VString* vs, const char* elem, size_t n)
{
  size_t need;
  assert(vs != NULL && elem != NULL);
  if(n == 0) {n = strlen(elem);}
  need = vs->length + n;
  if(vs->nonextendible) {
     /* Space must already be available */
      assert(vs->alloc >= need);
  } else {
      while(vs->alloc < need)
          vsexpand(vs);
  }
  memcpy(&vs->content[vs->length],elem,n);
  vs->length += n;
  if(!vs->nonextendible)
      vsnulterm(vs); /* guarantee nul term */
}

static void
vsappend(VString* vs, char elem)
{
  char s[2];
  s[0] = elem;
  s[1] = '\0';
  vsappendn(vs,s,1);
}

/**
Insert seq of elems s where |s|==n*elems into vs at position pos.
@param vs
@param pos where to insert; if pos > |vs->content| then expand vs.
@param elems to insert
@param elem no. of elems in elems
@return void
*/
static void
vsinsertn(VString* vs, size_t pos, const void* s, size_t slen)
{
    size_t totalspace = 0;
    size_t vslen = 0;

    assert(vs != NULL && s != NULL);
    if(slen == 0) {slen = strlen(s);}
    vslen = vslength(vs);
#if 0
initial: |len.........|
case 1:  |pos....||...slen...||...len - pos...|
case 2:  |pos==len....||...slen...|
case 3:  |len.........||...pos-len...||...slen...|
#endif
    if(pos < vslen) { /* Case 1 */
    /* make space for slen bytes at pos */
        totalspace = vslen+slen;
        vssetalloc(vs,(totalspace));
        vssetlength(vs,(totalspace));
        memmove((void*)(vs->content+pos+slen),(void*)(vs->content+pos),(vslen - pos));
        memcpy((void*)(vs->content+(pos)),(void*)s,slen);
    } else if(pos == vslen) { /* Case 2 */
    /* make space for slen bytes at pos (== length) */
        totalspace = vslen+slen;
        vssetalloc(vs,(totalspace));
        vssetlength(vs,(totalspace));
        /* append s at pos */
        memcpy((void*)(vs->content+(pos)),(void*)s,slen);     
    } else /*pos > vslen */ { /* Case 3 */
    /* make space for slen bytes at pos */
        totalspace = pos + slen;
        vssetalloc(vs,(totalspace));
        vssetlength(vs,(totalspace));
        /* clear space from length..pos */
        memset((void*)(vs->content+(vslen)),0,(pos-vslen));
        /* append s at pos */
        memmove((void*)(vs->content+((pos+slen))),(void*)(vs->content+(pos)),(vslen - pos));
        memcpy((void*)(vs->content+(pos)),(void*)s,slen);     
    }
    vsnulterm(vs);
}

/**
Remove seq of elems s where |s|==n*elems starting at position pos.
@param vs
@param pos where to remove; if pos > |vs->content| then do nothing
@param elide no. of elems remove
@return void
Side effect: reduce index by elided amount if index is past pos
*/
static void
vsremoven(VString* vs, size_t pos, size_t elide)
{
    size_t vslen = 0;
    size_t srcpos = 0;
    size_t srclen = 0; /* amount to memmove */

    assert(vs != NULL);
    if(elide == 0) goto done; /* nothing to move */
    vslen = vslength(vs);
    srcpos = pos + elide;
    /* edge cases */
    if(srcpos >= vslen) { /* remove everything from pos to vslen */
        vssetlength(vs,pos);
    } else {
	srclen = (vslen - srcpos);
	memmove((void*)(vscontents(vs)+(pos)),(void*)(vscontents(vs)+(srcpos)),srclen);
	vssetlength(vs,vslen - elide);
    }
done:
    vsnulterm(vs);
}

/**
Set the allocated capacity of the VString's capacity.
@param vs the array to expand
@param newalloc make sure alloc is at least this amount
@return void
*/
static void
vssetalloc(VString* vs, size_t newalloc)
{
    while(vs->alloc < newalloc) vsexpand(vs);
}

static void
vssetlength(VString* vs, size_t newlen)
{
    size_t oldlen;

    assert(vs != NULL);
    oldlen = vs->length;
    if(newlen > oldlen) {
        vssetalloc(vs,newlen+1);
    }
    vs->length = newlen;
    vsnulterm(vs);
}

/* Set unexpandible contents */
static void
vssetcontents(VString* vs, char* contents, size_t length)
{
    assert(vs != NULL && contents != NULL);
    vs->length = 0;
    if(!vs->nonextendible && vs->content != NULL) free(vs->content);
    vs->content = contents;
    vs->length = length;
    vs->alloc = length;
    vs->nonextendible = 1;
}

static char*
vsgetp(VString* vs, size_t pos)
{
    assert(vs->length >= pos);
    return vs->content + (pos);
}

/**
Shallow clone a VString object.
@param v the variable-length string to clone
@return ptr to clone
*/
static VString*
vsclone(VString* vs)
{
    VString* clone = NULL;
    clone = (VString*)calloc(1,sizeof(VString));
    assert(vs != NULL);
    *clone = *vs; /* copy the fields */
    if(clone->content != NULL) {
        clone->content = (char*)calloc(1,clone->alloc);
        assert(clone->content != NULL);
	if(clone->length > 0)
	    memcpy(clone->content,vs->content,clone->length+1);
	vsnulterm(clone);
    }
    return clone;
}

/* Extract the content and leave content null */
static char*
vsextract(VString* vs)
{
    char* x = NULL;
    if(vs == NULL) return NULL;
    if(vs->content == NULL) {
	/* guarantee content existence and nul terminated */
	if((vs->content = calloc(1,sizeof(char)))==NULL) return NULL;
	vs->length = 0;
    }
    x = vs->content;
    vs->content = NULL;
    vs->length = 0;
    vs->alloc = 0;
    return x;
}

static void
util_initialize(void)
{
    /* quiet compiler */
    void* f = NULL;
    f = f;
    f = (void*)vlistnew;
    f = (void*)vlistfree;
    f = (void*)vlistexpand;
    f = (void*)vlistget;
    f = (void*)vlistinsert;
    f = (void*)vlistremove;
    f = (void*)vlistpush;
    f = (void*)vlistfirst;
    f = (void*)vlistfreeall;
    f = (void*)vsnew;
    f = (void*)vsfree;
    f = (void*)vsexpand;
    f = (void*)vssetcontents;
    f = (void*)vsinsertn;
    f = (void*)vsremoven;
    f = (void*)vsappendn;
    f = (void*)vsappend;
    f = (void*)vsextract;
    f = (void*)vsclone;
    f = (void*)vsgetp;
    util_initialized = 1;
}

/* Following are always "in-lined"*/
#define vlistcontents(l)  ((l)==NULL?NULL:(l)->content)
#define vlistlength(l)  ((l)==NULL?0:(l)->length)
#define vlistclear(l)  vlistsetlength(l,0)
#define vlistsetlength(l,len)  do{if((l)!=NULL) (l)->length=len;} while(0)

#define vscontents(vs)  ((vs)==NULL?NULL:(vs)->content)
#define vslength(vs)  ((vs)==NULL?0:(vs)->length)
#define vscat(vs,s)  vsappendn(vs,s,0)
#define vsclear(vs)  vssetlength(vs,0)
#define vssetlength(vs,len)  do{if((vs)!=NULL) (vs)->length=len;} while(0)

#endif /*NCVUTIL_H*/
