/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#include <stdlib.h>
#include <curl/curl.h>

#include "netcdf.h"
#include "nclist.h"
#include "nchashmap.h"
#include "ast.h"
#include "ncStreamx.h"

/*Forward*/
static int nccr_walk_Group(Group*, NClist*);
static int nccr_walk_Dimension(Dimension*, NClist*);
static int nccr_walk_Variable(Variable*, NClist*);
static int nccr_walk_Structure(Structure*, NClist*);
static int nccr_walk_Attribute(Attribute*, NClist*);
static int nccr_walk_EnumTypedef(EnumTypedef*, NClist*);
static int nccr_walk_EnumType(EnumType*, NClist*);
static void crnote(void*, Notes**, NClist*);

int
nccr_cvtasterr(ast_err err)
{
    switch (err) {
    case AST_NOERR: return NC_NOERR;
    case AST_EOF: return NC_EINVAL;
    case AST_ENOMEM: return NC_ENOMEM;
    case AST_EFAIL: return NC_EINVAL /*generic*/;
    case AST_EIO: return NC_EIO;
    case AST_ECURL: return NC_ECURL;
    default: break;
    }
    return NC_EINVAL;
}

int
nccr_decodeheader(bytes_t* packet, size_t offset, Header** hdrp)
{
    ast_err status = AST_NOERR;
    ast_runtime* rt = NULL;
    Header* protohdr = NULL;

    /* Now decode the buffer */
    status = ast_byteio_new(AST_READ,packet->bytes+offset,packet->nbytes-offset,&rt);
    if(status != AST_NOERR) goto done;

    status = Header_read(rt,&protohdr);
    if(status != AST_NOERR) goto done;

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto done;

    if(hdrp) *hdrp = protohdr;

done:
    return nccr_cvtasterr(status);
}

/* Walk the Header tree to insert uid and sort capture all nodes */

static void
crnote(void* node, Notes** notesp, NClist* nodes)
{
    Notes* notes = NULL;
    if(nclistcontains(nodes,(ncelem)node)) return;
    if(*notesp != NULL) return;
    notes = (Notes*)calloc(1,sizeof(Notes));
    notes->uid = nclistlength(nodes);
    nclistpush(nodes,(ncelem)node);
    *notesp = notes;
}

int
nccr_walk_Header(Header* node, NClist* nodes)
{
    crnote(node,&node->notes.value,nodes);
    nccr_walk_Group(node->root,nodes);
}

static int
nccr_walk_Group(Group* node, NClist* nodes)
{
    int i;
    crnote(node,&node->notes.value,nodes);
    for(i=0;i<node->dims.count;i++)
        nccr_walk_Dimension(node->dims.values[i],nodes);
    for(i=0;i<node->vars.count;i++)
        nccr_walk_Variable(node->vars.values[i],nodes);
    for(i=0;i<node->structs.count;i++)
        nccr_walk_Structure(node->structs.values[i],nodes);
    for(i=0;i<node->atts.count;i++)
        nccr_walk_Attribute(node->atts.values[i],nodes);
    for(i=0;i<node->groups.count;i++)
        nccr_walk_Group(node->groups.values[i],nodes);
    for(i=0;i<node->enumTypes.count;i++)
        nccr_walk_EnumTypedef(node->enumTypes.values[i],nodes);
}

static int
nccr_walk_Dimension(Dimension* node, NClist* nodes)
{
    crnote(node,&node->notes.value,nodes);
}

static int
nccr_walk_Variable(Variable* node, NClist* nodes)
{
    int i;
    crnote(node,&node->notes.value,nodes);
    for(i=0;i<node->shape.count;i++)
        nccr_walk_Dimension(node->shape.values[i],nodes);
    for(i=0;i<node->atts.count;i++)
        nccr_walk_Attribute(node->atts.values[i],nodes);
}

static int
nccr_walk_Structure(Structure* node, NClist* nodes)
{
    int i;
    crnote(node,&node->notes.value,nodes);
    for(i=0;i<node->shape.count;i++)
        nccr_walk_Dimension(node->shape.values[i],nodes);
    for(i=0;i<node->atts.count;i++)
        nccr_walk_Attribute(node->atts.values[i],nodes);
    for(i=0;i<node->vars.count;i++)
        nccr_walk_Variable(node->vars.values[i],nodes);
    for(i=0;i<node->structs.count;i++)
        nccr_walk_Structure(node->structs.values[i],nodes);
}

static int
nccr_walk_Attribute(Attribute* node, NClist* nodes)
{
    crnote(node,&node->notes.value,nodes);
}

static int
nccr_walk_EnumTypedef(EnumTypedef* node, NClist* nodes)
{
    int i;
    crnote(node,&node->notes.value,nodes);
    for(i=0;i<node->map.count;i++)
        nccr_walk_EnumType(node->map.values[i],nodes);

}

static int
nccr_walk_EnumType(EnumType* node, NClist* nodes)
{
    crnote(node,&node->notes.value,nodes);
}



