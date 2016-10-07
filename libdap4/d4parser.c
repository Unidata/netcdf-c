/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include <stdarg.h>
#include "ezxml.h"

/**
 * Implement the Dap4 Parser Using a DOM Parser
 * 
 * This code creates in internal representation of the netcdf-4 metadata
 * to avoid having to make so many calls into the netcdf library.
 */

/***************************************************/

/*
Map an xml node name to an interpretation.
We use NCD4_NULL when we directly interpret
the tag and do not need to search for it.
E.g Dataset, Dim, econst, etc.
If the sort is NCD4_NULL, then that means it is
irrelevant because that keyword will never be
searched for in this table.
*/
struct KEYWORDINFO {
    char* tag; /* The xml tag e.g. <tag...> */
    NCD4sort sort; /* What kind of node are we building */
    nc_type subsort; /* discriminator */
    char* aliasfor; /* Some names are aliases for others */
} keywordmap[] = {
{"Attribute", NCD4_ATTR,NC_NAT,NULL},
{"Byte", NCD4_VAR,NC_BYTE,"Int8"},
{"Char", NCD4_VAR,NC_CHAR,NULL},
{"Dataset", NCD4_NULL,NC_NAT,NULL},
{"Dim", NCD4_NULL,NC_NAT,NULL},
{"Dimension", NCD4_DIM,NC_NAT,NULL},
{"Enum", NCD4_VAR,NC_ENUM,NULL},
{"Enumconst", NCD4_NULL,NC_NAT,NULL},
{"Enumeration", NCD4_TYPE,NC_ENUM,NULL},
{"Float32", NCD4_VAR,NC_FLOAT,NULL},
{"Float64", NCD4_VAR,NC_DOUBLE,NULL},
{"Group", NCD4_GROUP,NC_NAT,NULL},
{"Int16", NCD4_VAR,NC_SHORT,NULL},
{"Int32", NCD4_VAR,NC_INT,NULL},
{"Int64", NCD4_VAR,NC_INT64,NULL},
{"Int8", NCD4_VAR,NC_BYTE,NULL},
{"Map", NCD4_NULL,NC_NAT,NULL},
{"Opaque", NCD4_VAR,NC_OPAQUE,NULL},
{"OtherXML", NCD4_XML,NC_NAT,NULL},
{"Sequence", NCD4_VAR,NC_SEQ,NULL},
{"String", NCD4_VAR,NC_STRING,NULL},
{"Structure", NCD4_VAR,NC_STRUCT,NULL},
{"UByte", NCD4_VAR,NC_UBYTE,"UInt8"},
{"UInt16", NCD4_VAR,NC_USHORT,NULL},
{"UInt32", NCD4_VAR,NC_UINT,NULL},
{"UInt64", NCD4_VAR,NC_UINT64,NULL},
{"UInt8", NCD4_VAR,NC_UBYTE,NULL},
{"URL", NCD4_VAR,NC_STRING,NULL},
};
typedef struct KEYWORDINFO KEYWORDINFO;

static struct ATOMICTYPEINFO {
    char* name; nc_type type; size_t size;
} atomictypeinfo[] = {
/* Keep in sorted order for binary search */
{"Byte",NC_BYTE,sizeof(char)},
{"Char",NC_CHAR,sizeof(char)},
{"Float32",NC_FLOAT,sizeof(float)},
{"Float64",NC_DOUBLE,sizeof(double)},
{"Int16",NC_SHORT,sizeof(short)},
{"Int32",NC_INT,sizeof(int)},
{"Int64",NC_INT64,sizeof(long long)},
{"Int8",NC_BYTE,sizeof(char)},
{"String",NC_STRING,sizeof(char*)},
{"UByte",NC_UBYTE,sizeof(unsigned char)},
{"UInt16",NC_USHORT,sizeof(unsigned short)},
{"UInt32",NC_UINT,sizeof(unsigned int)},
{"UInt64",NC_UINT64,sizeof(unsigned long long)},
{"UInt8",NC_UBYTE,sizeof(unsigned char)},
{"URL",NC_STRING,sizeof(char*)},
{NULL,NC_NAT,0}
};

/***************************************************/

#ifdef D4DEBUG
static void setname(NCD4node* node, const char* name)
{
    nullfree(node->name);
    (node)->name = strdup(name); \
    fprintf(stderr,"setname: node=%lx name=%s\n",(unsigned long)(node),(node)->name); \
}
#define SETNAME(node,src) setname((node),src)
#else
#define SETNAME(node,src) do {nullfree((node)->name); (node)->name = strdup(src);} while(0);
#endif

/***************************************************/

/* Forwards */

static int addOrigType(NCD4parser*, NCD4node* src, NCD4node* dst);
static int buildAtomicTypes(NCD4parser*);
static int buildDapOpaque(NCD4parser*,NCD4node**);
static void classify(NCD4node* container, NCD4node* node);
static int convertString(union ATOMICS*, NCD4node* type, const char* s);
static int downConvert(union ATOMICS*, NCD4node* type);
static int fillgroup(NCD4parser*, NCD4node* group, ezxml_t xml);
static NCD4node* getOpaque(NCD4parser*, NCD4node*, ezxml_t varxml);
static int getValueStrings(NCD4parser*, ezxml_t xattr, NClist*);
static NCD4node* groupFor(NCD4node* node);
static KEYWORDINFO* keyword(const char* name);
static NCD4node* lookupAtomictype(NCD4parser*, const char* name);
static NCD4node* lookupFQN(NCD4parser*, const char* sfqn);
static NCD4node* lookupFQNList(NCD4parser*, NClist* fqn);
static NCD4node* lookupFQNListR(NCD4node* parent, NClist* fqn, int pos);
static NCD4node* makeAnonDim(NCD4parser*, const char* sizestr);
static int makeNode(NCD4parser*, NCD4node* parent, ezxml_t, NCD4sort, nc_type, NCD4node**);
static int parseAtomicVar(NCD4parser*, NCD4node* container, ezxml_t xml, NCD4node**);
static int parseAttributes(NCD4parser*, NCD4node* container, ezxml_t xml);
static int parseDimensions(NCD4parser*, NCD4node* group, ezxml_t xml);
static int parseDimRefs(NCD4parser*, NCD4node* var, ezxml_t xml);
static int parseEconsts(NCD4parser*, NCD4node* en, ezxml_t xml);
static int parseEnumerations(NCD4parser*, NCD4node* group, ezxml_t dom);
static int parseFields(NCD4parser*, NCD4node* container, ezxml_t xml);
static int parseError(NCD4parser*, ezxml_t errxml);
static int parseGroups(NCD4parser*, NCD4node* group, ezxml_t dom);
static int parseMaps(NCD4parser*, NCD4node* var, ezxml_t xml);
static int parseMetaData(NCD4parser*, NCD4node* node, ezxml_t xml);
static int parseStructure(NCD4parser*, NCD4node* container, ezxml_t dom, NCD4node**);
static int parseSequence(NCD4parser*, NCD4node* container, ezxml_t dom,NCD4node**);
static int parseLL(const char* text, long long*);
static int parseULL(const char* text, unsigned long long*);
static int parseVariables(NCD4parser*, NCD4node* group, ezxml_t xml);
static int parseVariable(NCD4parser*, NCD4node* group, ezxml_t xml, NCD4node**);
static void reclaimParser(NCD4parser* parser);
static void record(NCD4parser*, NCD4node* node);
static int splitOrigType(NCD4parser*, const char* fqn, NCD4node* var);
static void track(NCD4parser*, NCD4node* node);
static int traverse(NCD4parser*, ezxml_t dom);

/***************************************************/
/* API */

int
NCD4_parse(NCD4meta* metadata)
{
    int ret = NC_NOERR;
    NCD4parser* parser = NULL;
    int ilen;
    ezxml_t dom = NULL;

    /* Create and fill in the parser state */
    parser = (NCD4parser*)calloc(1,sizeof(NCD4parser));
    if(parser == NULL) {ret=NC_ENOMEM; goto done;}
    parser->metadata = metadata;
    ilen = strlen(parser->metadata->serial.dmr);
    dom = ezxml_parse_str(parser->metadata->serial.dmr,ilen);
    if(dom == NULL) {ret=NC_ENOMEM; goto done;}
    parser->types = nclistnew();
    parser->dims = nclistnew();
    parser->vars = nclistnew();
#ifdef D4DEBUG
    parser->debuglevel = 1;
#endif    

    /*Walk the DOM tree */
    ret = traverse(parser,dom);

done:
    if(ret != NC_NOERR) {
        reclaimParser(parser);
    }
    return THROW(ret);
}

static void
reclaimParser(NCD4parser* parser)
{
    if(parser == NULL) return;
    nclistfree(parser->atomictypes);
    nclistfree(parser->types);
    nclistfree(parser->dims);
    nclistfree(parser->vars);
    free (parser);
}

/**************************************************/

/* Recursively walk the DOM tree to create the metadata */
static int
traverse(NCD4parser* parser, ezxml_t dom)
{
    int ret = NC_NOERR;

    /* See if we have an <Error> or <Dataset> */
    if(strcmp(dom->name,"Error")==0) {
	ret=parseError(parser,dom);
	ret=NC_EDMR;
	goto done;
    } else if(strcmp(dom->name,"Dataset")==0) {
	const char* xattr = NULL;
        if((ret=makeNode(parser,NULL,NULL,NCD4_GROUP,NC_NULL,&parser->metadata->root))) goto done;
        parser->metadata->root->group.isdataset = 1;
        parser->metadata->root->meta.id = parser->metadata->ncid;
        SETNAME(parser->metadata->root,"/");
	xattr = ezxml_attr(dom,"name");
	if(xattr != NULL) parser->metadata->root->group.datasetname = strdup(xattr);
	xattr = ezxml_attr(dom,"dapVersion");
	if(xattr != NULL) parser->metadata->root->group.dapversion = strdup(xattr);
	xattr = ezxml_attr(dom,"dmrVersion");
	if(xattr != NULL) parser->metadata->root->group.dmrversion = strdup(xattr);
        /* fill in the atomic types */
        if((ret=buildAtomicTypes(parser))) goto done;
        /* Recursively walk the tree */
        if((ret = fillgroup(parser,parser->metadata->root,dom))) goto done;
    } else
	FAIL(NC_EINVAL,"Unexpected dom root name: %s",dom->name);
done:
    return THROW(ret);
}

static int
fillgroup(NCD4parser* parser, NCD4node* group, ezxml_t xml)
{
    int ret = NC_NOERR;

    /* Extract Dimensions */
    if((ret = parseDimensions(parser,group,xml))) goto done;
    /* Extract Enum types */
    if((ret = parseEnumerations(parser,group,xml))) goto done;
    /* Extract variables */
    if((ret = parseVariables(parser,group,xml))) goto done;
    /* Extract subgroups*/
    if((ret = parseGroups(parser,group,xml))) goto done;
    /* Parse group level attributes */
    if((ret = parseAttributes(parser,group,xml))) goto done;    
done:
    return THROW(ret);
}

static int
parseDimensions(NCD4parser* parser, NCD4node* group, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;
    for(x=ezxml_child(xml, "Dimension");x != NULL;x = ezxml_next(x)) {
	NCD4node* dimnode = NULL;
	NCD4node* attr = NULL;
	const char* sizestr;
	unsigned long long size;
	sizestr = ezxml_attr(x,"size");
	if(sizestr == NULL)
	    FAIL(NC_EDIMSIZE,"Dimension has no size");
	if((ret = parseULL(sizestr,&size))) goto done;
	if((ret=makeNode(parser,group,x,NCD4_DIM,NC_NULL,&dimnode))) goto done;
	dimnode->dim.size = (long long)size;
	/* Process attributes */
	if((ret = parseAttributes(parser,dimnode,x))) goto done;    
	/* See if this was intended as an unlimited */
	attr = NCD4_findAttr(dimnode,UCARTAGUNLIM);
	if(attr != NULL)
	    dimnode->nc4.isunlim = 1;
	classify(group,dimnode);
    }
done:
    return THROW(ret);
}

static int
parseEnumerations(NCD4parser* parser, NCD4node* group, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;

    for(x=ezxml_child(xml, "Enumeration");x != NULL;x = ezxml_next(x)) {
	NCD4node* node = NULL;
	NCD4node* basetype = NULL;
	const char* fqn = ezxml_attr(x,"basetype");
	basetype = lookupFQN(parser,fqn);
	if(basetype == NULL) {
	    FAIL(NC_EBADTYPE,"Enumeration has unknown type: ",fqn);
	}
	if((ret=makeNode(parser,group,x,NCD4_TYPE,NC_ENUM,&node))) goto done;
	node->basetype = basetype;
	if((ret=parseEconsts(parser,node,x))) goto done;
	if(nclistlength(node->en.econsts) == 0)
	    FAIL(NC_EINVAL,"Enumeration has no values");
	classify(group,node);
	/* Finally, see if this type has UCARTAGORIGTYPE xml attribute */
	
    }
done:
    return THROW(ret);
}

static int
parseEconsts(NCD4parser* parser, NCD4node* en, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;
    NClist* econsts = nclistnew();

    for(x=ezxml_child(xml, "EnumConst");x != NULL;x = ezxml_next(x)) {
        NCD4node* ec = NULL;
	const char* name;
	const char* svalue;
	name = ezxml_attr(x,"name");
	if(name == NULL) FAIL(NC_EBADNAME,"Enum const with no name");
	if((ret=makeNode(parser,en,x,NCD4_ECONST,NC_NULL,&ec))) goto done	;
	svalue = ezxml_attr(x,"value");
	if(svalue == NULL)
	    FAIL(NC_EINVAL,"Enumeration Constant has no value");
	if((ret=convertString(&ec->en.ecvalue,en->basetype,svalue)))
	    FAIL(NC_EINVAL,"Non-numeric Enumeration Constant: %s->%s",ec->name,svalue);
	PUSH(econsts,ec);	
    }
    en->en.econsts = econsts;
done:
    return THROW(ret);
}

static int
parseVariables(NCD4parser* parser, NCD4node* group, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;
    for(x=xml->child;x != NULL;x=x->ordered) {
	NCD4node* node = NULL;
	KEYWORDINFO* info = keyword(x->name);
	if(info == NULL)
	    FAIL(NC_ETRANSLATION,"Unexpected node type: %s",x->name);
	/* Check if we need to process this node */
	if(!ISVAR(info->sort)) continue; /* Handle elsewhere */
	node = NULL;
	ret = parseVariable(parser,group,x,&node);
        if(ret != NC_NOERR || node == NULL) break;
    }
done:
    return THROW(ret);
}

static int
parseVariable(NCD4parser* parser, NCD4node* container, ezxml_t xml, NCD4node** nodep)
{
    int ret = NC_NOERR;
    NCD4node* node = NULL;
    KEYWORDINFO* info = keyword(xml->name);

    switch (info->subsort) {
    case NC_STRUCT:
	ret = parseStructure(parser,container,xml,&node);
	break;
    case NC_SEQ:
	ret = parseSequence(parser,container,xml,&node);
	break;
    default:
	ret = parseAtomicVar(parser,container,xml,&node);
    }
    if(node != NULL) 
        classify(container,node);
    *nodep = node;

    return THROW(ret);
}

static int
parseMetaData(NCD4parser* parser, NCD4node* container, ezxml_t xml)
{
    int ret = NC_NOERR;
    /* Process dimrefs */
    if((ret=parseDimRefs(parser,container,xml))) goto done;
    /* Process attributes */
    if((ret = parseAttributes(parser,container,xml))) goto done;    
    /* Process maps */
    if((ret = parseMaps(parser,container,xml))) goto done;    
done:
    return THROW(ret);
}

static int
parseStructure(NCD4parser* parser, NCD4node* container, ezxml_t xml, NCD4node** nodep)
{
    int ret = NC_NOERR;
    NCD4node* var = NULL;
    NCD4node* type = NULL;
    NCD4node* group = NULL;
    NCbytes* fqn = ncbytesnew();
    char* name = NULL;

    group = groupFor(container); /* default: put type in the same group as container */

    /* Make the structure as a type with (for now) same name as the variable */
    if((ret=makeNode(parser,group,xml,NCD4_TYPE,NC_STRUCT,&type))) goto done;
    classify(group,type);

    /* Also make the structure as a variable; will be classified later */
    if((ret=makeNode(parser,container,xml,NCD4_VAR,NC_STRUCT,&var))) goto done;
    /* Set the basetype */
    var->basetype = type;

    /* Create the typename as the last part of the fqn of the var name */
    name = NCD4_makeName(var,"_");
    if(name == NULL)
	FAIL(NC_ENOMEM,"Out of memory");
    SETNAME(type,name);

    /* Parse Fields into the type */
    if((ret = parseFields(parser,type,xml))) goto done;    

    /* Parse attributes, dims, and maps into the var */
    if((ret = parseMetaData(parser,var,xml))) goto done;

    record(parser,var);

    /* See if this var has UCARTAGORIGTYPE attribute, but place it on the type */
    if((ret=addOrigType(parser,var,type))) goto done;

    if(nodep) *nodep = var;

done:
    nullfree(name);
    ncbytesfree(fqn);
    return THROW(ret);
}

static int
parseFields(NCD4parser* parser, NCD4node* container, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;
    for(x=xml->child;x != NULL;x=x->ordered) {
	NCD4node* node = NULL;
        KEYWORDINFO* info = keyword(x->name);
	if(!ISVAR(info->sort)) continue; /* not a field */
	ret = parseVariable(parser,container,x,&node);
	if(ret) goto done;
    }
done:
    return THROW(ret);
}

static int
parseSequence(NCD4parser* parser, NCD4node* container, ezxml_t xml, NCD4node** nodep)
{
    int ret = NC_NOERR;
    NCD4node* var = NULL;
    NCD4node* type = NULL;
    NCD4node* group = NULL;
    NCbytes* fqn = ncbytesnew();
    char* name = NULL;

    group = groupFor(container);

    /* Make the sequence as a type with (for now) same name as the variable */
    if((ret=makeNode(parser,group,xml,NCD4_TYPE,NC_SEQ,&type))) goto done;
    classify(group,type);

    /* Also make the sequence as a variable; will be classified later */
    if((ret=makeNode(parser,container,xml,NCD4_VAR,NC_SEQ,&var))) goto done;
    /* Set the basetype */
    var->basetype = type;

    /* Create the typename as the last part of the fqn of the name */
    name = NCD4_makeName(var,"_");
    if(name == NULL)
	FAIL(NC_ENOMEM,"Out of memory");
    SETNAME(type,name);

    /* Parse Fields into type */
    if((ret = parseFields(parser,type,xml))) goto done;    

    /* Parse attributes, dims, and maps into var*/
    if((ret = parseMetaData(parser,var,xml))) goto done;

    record(parser,var);

    /* See if this var has UCARTAGORIGTYPE attribute, but place it on the type */
    if((ret=addOrigType(parser,var,type))) goto done;

    if(nodep) *nodep = var;

done:
    nullfree(name);
    ncbytesfree(fqn);
    return THROW(ret);
}

static int
parseGroups(NCD4parser* parser, NCD4node* parent, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;
    for(x=ezxml_child(xml, "Group");x != NULL;x = ezxml_next(x)) {
	NCD4node* group = NULL;
	const char* name = ezxml_attr(x,"name");
	if(name == NULL) FAIL(NC_EBADNAME,"Group has no name");
	if((ret=makeNode(parser,parent,x,NCD4_GROUP,NC_NULL,&group))) goto done;
        if((ret = fillgroup(parser,group,x))) goto done;
        /* Parse group attributes */
        if((ret = parseAttributes(parser,group,x))) goto done;    
	PUSH(parent->groups,group);
    }
done:
    return THROW(ret);
}

static int
parseAtomicVar(NCD4parser* parser, NCD4node* group, ezxml_t xml, NCD4node** nodep)
{
    int ret = NC_NOERR;
    NCD4node* node = NULL;
    NCD4node* base = NULL;
    const char* typename;
    KEYWORDINFO* info;
   
    /* Check for aliases */
    for(typename=xml->name;;) {
	info = keyword(typename);
	if(info->aliasfor == NULL) break;
	typename = info->aliasfor;
    }	
    /* Locate its basetype; handle opaque and enum separately */
    if(info->subsort == NC_ENUM) {
        const char* enumfqn = ezxml_attr(xml,"enum");
	if(enumfqn == NULL)
	    base = NULL;
	else
	    base = lookupFQN(parser,enumfqn);
    } else if(info->subsort == NC_OPAQUE) {
	base = getOpaque(parser,group,xml);
    } else {
	base = lookupFQN(parser,info->tag);
    }
    if(base == NULL || !ISTYPE(base->sort)) {
	FAIL(NC_EBADTYPE,"Unexpected variable type: %s",info->tag);
    }
    if((ret=makeNode(parser,group,xml,NCD4_VAR,base->subsort,&node))) goto done;
    node->basetype = base;
    /* Parse attributes, dims, and maps */
    if((ret = parseMetaData(parser,node,xml))) goto done;    
    /* See if this var has UCARTAGORIGTYPE attribute */
    if((ret=addOrigType(parser,node,node))) goto done;
    if(nodep) *nodep = node;
done:
    return THROW(ret);
}

static int
parseDimRefs(NCD4parser* parser, NCD4node* var, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;
    for(x=ezxml_child(xml, "Dim");x!= NULL;x=ezxml_next(x)) {
	NCD4node* dim = NULL;
	const char* fqn;

	fqn = ezxml_attr(x,"name");
	if(fqn != NULL) {
   	    dim = lookupFQN(parser,fqn);
	    if(!ISDIM(dim->sort)) {
	        FAIL(NC_EBADDIM,"Cannot locate dim with name: %s",fqn);
	    }
	} else {
	    const char* sizestr = ezxml_attr(x,"size");
	    if(sizestr == NULL) {
	        ERROR(NC_EBADDIM,"Dimension reference has no name and no size");
		goto done;
	    }
	    /* Make or reuse anonymous dimension in root group */
	    dim = makeAnonDim(parser,sizestr);
	    if(dim == NULL)
		FAIL(NC_EBADDIM,"Cannot create anonymous dimension for size: %s",sizestr);
	}
	PUSH(var->dims,dim);
    }
done:
    return THROW(ret);
}

static int
parseMaps(NCD4parser* parser, NCD4node* var, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;

    for(x=ezxml_child(xml, "Map");x!= NULL;x=ezxml_next(x)) {
	NCD4node* mapref = NULL;
	const char* fqn;
	fqn = ezxml_attr(x,"name");
	if(fqn == NULL)
	    FAIL(NC_ENOTVAR,"<Map> has no name attribute");
        mapref = lookupFQN(parser,fqn);
	if(mapref == NULL)
	    FAIL(NC_ENOTVAR,"<Map> name does not refer to a variable: %s",fqn);
        if(!ISVAR(mapref->sort))
	    FAIL(NC_ENOTVAR,"Cannot locate map variable with name: %s",fqn);
	PUSH(var->maps,mapref);
    }
done:
    return THROW(ret);
}

static int
parseAttributes(NCD4parser* parser, NCD4node* container, ezxml_t xml)
{
    int ret = NC_NOERR;
    ezxml_t x;
    for(x=ezxml_child(xml, "Attribute");x!= NULL;x=ezxml_next(x)) {
	const char* name = ezxml_attr(x,"name");
	const char* type = ezxml_attr(x,"type");
	NCD4node* attr = NULL;
	NCD4node* basetype;
	NClist* values = NULL;

	if(name == NULL) FAIL(NC_EBADNAME,"Missing <Attribute> name");
	if(type == NULL) FAIL(NC_EBADTYPE,"Missing <Attribute> type");
	if((ret=makeNode(parser,container,x,NCD4_ATTR,NC_NULL,&attr))) goto done;
	/* HACK: If the attribute is _FillValue, then force the use of the
           container's type as the attribute type */
	if(strcmp(attr->name,"_FillValue") == 0)
	    basetype = container->basetype;
	else
	    basetype = lookupFQN(parser,type);
	if(basetype == NULL)
	    FAIL(NC_EBADTYPE,"Unknown <Attribute> type: ",type);
	if(basetype->subsort == NC_NAT && basetype->subsort != NC_ENUM)
	    FAIL(NC_EBADTYPE,"<Attribute> type must be atomic or enum: ",type);
	attr->basetype = basetype;
	values = nclistnew();
	if((ret=getValueStrings(parser,x,values))) {
	    nclistfreeall(values);
	    FAIL(NC_EINVAL,"Malformed attribute: %s",name);
	}
	attr->attr.values = values;
	PUSH(container->attributes,attr);
    }
done:
    return THROW(ret);
}

static int
parseError(NCD4parser* parser, ezxml_t errxml)
{
    const char* shttpcode = ezxml_attr(errxml,"httpcode");
    ezxml_t x;
    if(shttpcode == NULL) shttpcode = "400";
    if(sscanf(shttpcode,"%d",&parser->metadata->error.httpcode) != 1)
        nclog(NCLOGERR,"Malformed <ERROR> response");
    x=ezxml_child(errxml, "Message");
    if(x != NULL) {
	const char* txt = ezxml_txt(x);
	parser->metadata->error.message = (txt == NULL ? NULL : strdup(txt));
    }
    x=ezxml_child(errxml, "Context");
    if(x != NULL) {
	const char* txt = ezxml_txt(x);
	parser->metadata->error.context = (txt == NULL ? NULL : strdup(txt));
    }
    x=ezxml_child(errxml, "OtherInformation");
    if(x != NULL) {
	const char* txt = ezxml_txt(x);
	parser->metadata->error.otherinfo = (txt == NULL ? NULL : strdup(txt));
    }
    return NC_NOERR;
}

/*
Find or create an opaque type
*/
static NCD4node*
getOpaque(NCD4parser* parser, NCD4node* var, ezxml_t varxml)
{
    int i, ret = NC_NOERR;
    long long len;
    NCD4node* opaquetype = NULL;
    NCD4node* ucar = NULL;
    const char* s;

    /* See if this var has UCARTAGOPAQUE attribute */
    if((ucar=NCD4_findAttr(var,UCARTAGOPAQUE)) != NULL) {
	if(nclistlength(ucar->attr.values) > 0) {
            s = nclistget(ucar->attr.values,0);
            if((ret = parseLL(s,&len)) || (len < 0))
	       FAIL(NC_EINVAL,"Illegal opaque len: %s",s);
	}
    } else
	len = 0;
    if(opaquetype == NULL) {
        /* Try to locate existing opaque type with this length */
        for(i=0;i<nclistlength(parser->types); i++) {
	    NCD4node* op = (NCD4node*)nclistget(parser->types,i);
	    if(op->subsort != NC_OPAQUE) continue;
	    if(op->opaque.size == len) {opaquetype = op; break;}
	}
    }
    if(opaquetype == NULL) {/* create it */
	if(len > 0) {
	    char name[NC_MAX_NAME+1];
	    NCD4node* group = groupFor(var);
	    /* Make name be "opaqueN" */
	    snprintf(name,NC_MAX_NAME,"opaque%d_t",len);
	    if(group == NULL)
	        FAIL(NC_EINVAL,"Cannot locate group for: %s",var->name);
	    /* Opaque types are always created in the current group */
	    if((ret=makeNode(parser,group,NULL,NCD4_TYPE,NC_OPAQUE,&opaquetype)))
	        goto done;
  	    SETNAME(opaquetype,name);
	    opaquetype->opaque.size = len;
        } else { /* len == 0 */
	    if((ret=buildDapOpaque(parser,&opaquetype))) goto done;
        }
	if(opaquetype != NULL) {
	    record(parser,opaquetype);			
	}
    }
done:
    return opaquetype;        
}

/* get all value strings */
static int
getValueStrings(NCD4parser* parser, ezxml_t xattr, NClist* svalues)
{
    const char* s;
    /* See first if we have a "value" xml attribute */
    s = ezxml_attr(xattr,"value");
    if(s != NULL)
	PUSH(svalues,strdup(s));
    else {/* look for <Value> subnodes */
	ezxml_t x;
        for(x=ezxml_child(xattr, "Value");x != NULL;x = ezxml_next(x)) {
	    /* case 1: value= xml attribute */
	    s = ezxml_attr(x,"value");
	    if(s == NULL) {/* Get the text */
		s = x->txt;
		if(s == NULL) s = "";
	    }
	    PUSH(svalues,strdup(s));
	}
    }
    return NC_NOERR;
}

/***************************************************/
/* Utilities */

/* If a node has the UCARTAGORIGTYPE attribute, then capture that annotation. */
static int
addOrigType(NCD4parser* parser, NCD4node* src, NCD4node* dst)
{
    int ret = NC_NOERR;
    NCD4node* ucar;

    if(dst == NULL) dst = src;
    if((ucar=NCD4_findAttr(src,UCARTAGORIGTYPE)) != NULL) {
	if(nclistlength(ucar->attr.values) > 0) {
	    /* Record the original type in the destination*/
	    if((ret=splitOrigType(parser,(char*)nclistget(ucar->attr.values,0),dst))) goto done;
	}
    }
done:
    return ret;
}

static int
splitOrigType(NCD4parser* parser, const char* fqn, NCD4node* type)
{
    int ret = NC_NOERR;
    NClist* pieces = nclistnew();
    NCD4node* group = NULL;
    char* name = NULL;

    if((ret=NCD4_parseFQN(fqn,pieces))) goto done;
    /* It should be the case that the pieces are .../group/name */
    name = (char*)nclistpop(pieces);
    group = lookupFQNList(parser,pieces);
    if(group == NULL) {
	FAIL(NC_ENOGRP,"Non-existent group in FQN: ",fqn);    
    } else if(!ISGROUP(group->sort)) {
	FAIL(NC_EINVAL,"Malformed FQN: ",fqn);    	
    }
    type->nc4.orig.name = strdup(name);
    type->nc4.orig.group = group;

done:
    return THROW(ret);
}

/* Locate an attribute.
   If not found, then *attrp will be null
*/
NCD4node*
NCD4_findAttr(NCD4node* container, const char* attrname)
{
    int i;
    /* Look directly under this xml for <Attribute> */
    for(i=0;i<nclistlength(container->attributes);i++) {
	NCD4node* attr = (NCD4node*)nclistget(container->attributes,i);
	if(strcmp(attr->name,attrname)!=0) continue;
	return attr;
    }    
    return NULL;
}

static NCD4node*
groupFor(NCD4node* node)
{
    while(node->sort != NCD4_GROUP) node = node->container;
    return node;
}

/*
Parse a simple string of digits into an unsigned long long
Return the value.
*/

static int
parseULL(const char* text, unsigned long long* ullp)
{
    extern int errno;
    char* endptr;
    unsigned long long uint64 = 0;

    errno = 0; endptr = NULL;
#ifdef HAVE_STRTOULL
    uint64 = strtoull(text,&endptr,10);
    if(errno == ERANGE)
	return NC_ERANGE;
#else /*!(defined HAVE_STRTOLL && defined HAVE_STRTOULL)*/
    sscanf((char*)text, "%llu", &uint64);
    /* Have no useful way to detect out of range */
#endif /*!(defined HAVE_STRTOLL && defined HAVE_STRTOULL)*/
    if(ullp) *ullp = uint64;
    return NC_NOERR;
}

/*
Parse a simple string of digits into an signed long long
Return the value.
*/

static int
parseLL(const char* text, long long* llp)
{
    extern int errno;
    char* endptr;
    long long int64 = 0;

    errno = 0; endptr = NULL;
#ifdef HAVE_STRTOLL
    int64 = strtoll(text,&endptr,10);
    if(errno == ERANGE)
	return NC_ERANGE;
#else /*!(defined HAVE_STRTOLL && defined HAVE_STRTOLL)*/
    sscanf((char*)text, "%lld", &int64);
    /* Have no useful way to detect out of range */
#endif /*!(defined HAVE_STRTOLL && defined HAVE_STRTOLL)*/
    if(llp) *llp = int64;
    return NC_NOERR;
}

static NCD4node*
lookupFQNList(NCD4parser* parser, NClist* fqn)
{
    NCD4node* root = parser->metadata->root;
    /* verify root */
    if(strcmp(root->name,nclistget(fqn,0))!=0) {
	ERROR(NC_EBADID,"Root group is not named '/': ",root->name);
	return NULL;
    }
    return lookupFQNListR(root,fqn,1); /* start at pos 1 to skip root */
}

/*
Works only for dimensions, types, groups, and variables;
not attributes or enum consts.
*/ 
static NCD4node*
lookupFQNListR(NCD4node* parent, NClist* fqn, int pos)
{
    int i, len;
    char* name;

    /* Termination checks */
    len = nclistlength(fqn);
    if(pos >= len)
	return NULL;
    if(parent->sort != NCD4_GROUP) {
	ERROR(NC_EINVAL,"fqn is not toplevel in group");
	goto done;
    }
    name = nclistget(fqn,pos);
    /* Search parent for matching name */
    for(i=0;i<nclistlength(parent->group.elements);i++) {
	NCD4node* node = (NCD4node*)nclistget(parent->group.elements,i);
	if(strcmp(node->name,name)==0) {
	    if(pos == (len-1))
	        return node;
	    else
	        return lookupFQNListR(node,fqn,pos+1);	
	}
    }
done:
    return NULL; /* not found */
}

static NCD4node*
lookupFQN(NCD4parser* parser, const char* sfqn)
{
    int ret = NC_NOERR;
    NClist* fqnlist = nclistnew();
    NCD4node* match = NULL;

    /* Short circuit atomic types */
    match = lookupAtomictype(parser,(sfqn[0]=='/'?sfqn+1:sfqn));
    if(match != NULL)
	return match;
    if((ret=NCD4_parseFQN(sfqn,fqnlist))) goto done;
    match = lookupFQNList(parser,fqnlist);    
done:
    nclistfreeall(fqnlist);
    return match;
}

static KEYWORDINFO*
keyword(const char* name)
{
    int n = sizeof(keywordmap)/sizeof(KEYWORDINFO);
    int L = 0;
    int R = (n - 1);
    for(;;) {
	if(L > R) break;
        int m = (L + R) / 2;
	struct KEYWORDINFO* p = &keywordmap[m];
	int cmp = strcasecmp(p->tag,name);
	if(cmp == 0) return p;
	if(cmp < 0)
	    L = (m + 1);
	else /*cmp > 0*/
	    R = (m - 1);
    }
    return NULL;
}

static int
buildDapOpaque(NCD4parser* parser, NCD4node** opp)
{
    /* Construct a single opaque type for mapping DAP opaque type */
    NCD4node* op;
    int ret = makeNode(parser,parser->metadata->root,NULL,NCD4_TYPE,NC_OPAQUE,&op);
    if(ret == NC_NOERR) {
	SETNAME(op,"_bytestring");
	op->opaque.size = 0;
	if(opp) *opp = op;
	PUSH(parser->metadata->root->types,op);
    }
    return THROW(ret);
}

static int
buildAtomicTypes(NCD4parser* parser)
{
    int ret = NC_NOERR;
    NCD4node* node;
    struct ATOMICTYPEINFO* ati;
    
    parser->atomictypes = nclistnew();
    if(parser->atomictypes == NULL)
	return NC_ENOMEM;
    for(ati=atomictypeinfo;ati->name;ati++) {
        if((ret=makeNode(parser,parser->metadata->root,NULL,NCD4_TYPE,ati->type,&node))) goto done;
	SETNAME(node,ati->name);
        node->container = parser->metadata->root;
	record(parser,node);
	PUSH(parser->atomictypes,node);
    }

done:
    return THROW(ret);
}

/* Binary search the set of set of atomictypes */
static NCD4node*
lookupAtomictype(NCD4parser* parser, const char* name)
{
    int n = nclistlength(parser->atomictypes);
    int L = 0;
    int R = (n - 1);
    NCD4node* p;

    for(;;) {
	if(L > R) break;
        int m = (L + R) / 2;
	p = (NCD4node*)nclistget(parser->atomictypes,m);
	int cmp = strcasecmp(p->name,name);
	if(cmp == 0) return p;
	if(cmp < 0)
	    L = (m + 1);
	else /*cmp > 0*/
	    R = (m - 1);
    }
    return NULL;
}

/**************************************************/

static int
makeNode(NCD4parser* parser, NCD4node* parent, ezxml_t xml, NCD4sort sort, nc_type subsort, NCD4node** nodep)
{
    int ret = NC_NOERR;
    NCD4node* node = (NCD4node*)calloc(1,sizeof(NCD4node));
    if(node == NULL) return NC_ENOMEM;
    node->sort = sort;
    node->subsort = subsort;
    node->container = parent;
    /* Set node name, if it exists */
    if(xml != NULL) {
        const char* name = ezxml_attr(xml,"name");
        if(name != NULL) {
	    if(strlen(name) > NC_MAX_NAME) {
	        nclog(NCLOGERR,"Name too long: %s",name);
	    }
	    SETNAME(node,name);
	}
    }
    if(parent != NULL) {
	if(parent->sort == NCD4_GROUP)
	    PUSH(parent->group.elements,node);
    }
    track(parser,node);
    if(nodep) *nodep = node;
    return THROW(ret);
}

static NCD4node*
makeAnonDim(NCD4parser* parser, const char* sizestr)
{
    long long size = 0;
    int ret;
    char name[NC_MAX_NAME+1];
    NCD4node* dim = NULL;
    NCD4node* root = parser->metadata->root;

    ret = parseLL(sizestr,&size);
    if(ret) return NULL;
    snprintf(name,NC_MAX_NAME,"_Anonymous%lld",size);
    /* See if it exists already */
    dim = lookupFQN(parser,name);
    if(dim == NULL) {/* create it */
	if((ret=makeNode(parser,root,NULL,NCD4_DIM,NC_NULL,&dim))) goto done;
	SETNAME(dim,name);
	dim->dim.size = (long long)size;
	PUSH(root->dims,dim);
    }
done:
    return (ret?NULL:dim);
}

static void
classify(NCD4node* container, NCD4node* node)
{
    switch (node->sort) {
    case NCD4_GROUP:
	PUSH(container->groups,node);
        break;
    case NCD4_DIM:
	PUSH(container->dims,node);
        break;
    case NCD4_TYPE:
	PUSH(container->types,node);
        break;
    case NCD4_VAR:
	PUSH(container->vars,node);
        break;
    case NCD4_ATTR: case NCD4_XML:
	PUSH(container->attributes,node);
        break;
    default: break;
    }
}

static void
record(NCD4parser* parser, NCD4node* node)
{
    switch (node->sort) {
    case NCD4_GROUP:
	PUSH(parser->groups,node);
        break;
    case NCD4_DIM:
	PUSH(parser->dims,node);
        break;
    case NCD4_TYPE:
	PUSH(parser->types,node);
        break;
    case NCD4_VAR:
	PUSH(parser->vars,node);
        break;
    default: break;
    }
}

static void
track(NCD4parser* parser, NCD4node* node)
{
#ifdef D4DEBUG
    fprintf(stderr,"track: node=%lx sort=%d subsort=%d",(unsigned long)node,node->sort,node->subsort);
    if(node->name != NULL)
        fprintf(stderr," name=%s\n",node->name);
    fprintf(stderr,"\n");
#endif
    PUSH(parser->metadata->allnodes,node);
#ifdef D4DEBUG
    fprintf(stderr,"track: |allnodes|=%ld\n",nclistlength(parser->metadata->allnodes));
    fflush(stderr);
#endif
}

/**************************************************/

static int
convertString(union ATOMICS* converter, NCD4node* type, const char* s)
{
    switch (type->subsort) {
    case NC_BYTE:
    case NC_SHORT:
    case NC_INT:
    case NC_INT64:
	if(sscanf(s,"%lld",&converter->i64) != 1) return NC_ERANGE;
	break;
    case NC_UBYTE:
    case NC_USHORT:
    case NC_UINT:
    case NC_UINT64:
	if(sscanf(s,"%llu",&converter->u64) != 1) return NC_ERANGE;
	break;
    case NC_FLOAT:
    case NC_DOUBLE:
	if(sscanf(s,"%lf",&converter->f64) != 1) return NC_ERANGE;
	break;
    case NC_STRING:
	converter->s[0]= strdup(s);
	break;
    }/*switch*/
    return downConvert(converter,type);
}

static int
downConvert(union ATOMICS* converter, NCD4node* type)
{
    unsigned long long u64 = converter->u64[0];
    long long i64 = converter->i64[0];
    double f64 = converter->f64[0];
    char* s = converter->s[0];
    switch (type->subsort) {
    case NC_BYTE:
	converter->i8[0] = (char)i64;
	break;
    case NC_UBYTE:
	converter->u8[0] = (unsigned char)u64;
	break;
    case NC_SHORT:
	converter->i16[0] = (short)i64;
	break;
    case NC_USHORT:
	converter->u16[0] = (unsigned short)u64;
	break;
    case NC_INT:
	converter->i32[0] = (int)i64;
	break;
    case NC_UINT:
	converter->u32[0] = (unsigned int)u64;
	break;
    case NC_INT64:
	converter->i64[0] = i64;
	break;
    case NC_UINT64:
	converter->u64[0]= u64;
	break;
    case NC_FLOAT:
	converter->f32[0] = (float)f64;
	break;
    case NC_DOUBLE:
	converter->f64[0] = f64;
	break;
    case NC_STRING:
	converter->s[0]= s;
	break;
    }/*switch*/
    return NC_NOERR;
}

