/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#include <curl/curl.h>

#include "netcdf.h"
#include "nclist.h"
#include "nchashmap.h"
#include "ast.h"
#include "ncstreamx.h"

/*Forward*/
static int nccr_walk_Group(Group*, NClist*, NChashmap*);
static int nccr_walk_Dimension(Dimension*, NClist*, NChashmap*);
static int nccr_walk_Variable(Variable*, NClist*, NChashmap*);
static int nccr_walk_Structure(Structure*, NClist*, NChashmap*);
static int nccr_walk_Attribute(Attribute*, NClist*, NChashmap*);
static int nccr_walk_EnumTypedef(EnumTypedef*, NClist*, NChashmap*);
static int nccr_walk_EnumType(EnumType*, NClist*, NChashmap*);
static void crnote(Notes*, NClist*, NChashmap*);

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
nccr_decodeheaderdata(bytes_t* packet, Header** hdrp)
{
    ast_err status = AST_NOERR;
    ast_runtime* rt = NULL;
    Header* protohdr = NULL;

    /* Now decode the buffer */
    status = ast_byteio_new(AST_READ,packet->bytes,packet->nbytes,&rt);
    if(status != AST_NOERR) goto done;

    status = Header_read(rt,&protohdr);
    if(status != AST_NOERR)
	fprintf(stderr,"*** FAIL: Header decode failed\n");

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto done;

    if(hdrp) *hdrp = protohdr;

done:
    return nccr_cvtasterr(status);
}

/* Walk the Header tree to insert uid and sort capture all nodes */

static void
crnote(Notes* node, NClist* nodes, NChashmap* map)
{
    if(nclistcontains(nodes,(ncelem)node)) return;
    node->uid = nclistlength(nodes);
    nclistpush(nodes,(ncelem)node);
    nchashinsert(map,(nchashid)node->uid,(ncelem)node);
}

int
nccr_walk_Header(Header* node, NClist* nodes, NChashmap* map)
{
    crnote((Notes*)node,nodes,map);
    nccr_walk_Group(node->root,nodes,map);
}

static int
nccr_walk_Group(Group* node, NClist* nodes, NChashmap* map)
{
    int i;
    crnote((Notes*)node,nodes,map);
    for(i=0;i<node->dims.count;i++)
        nccr_walk_Dimension(node->dims.values[i],nodes,map);
    for(i=0;i<node->vars.count;i++)
        nccr_walk_Variable(node->vars.values[i],nodes,map);
    for(i=0;i<node->structs.count;i++)
        nccr_walk_Structure(node->structs.values[i],nodes,map);
    for(i=0;i<node->atts.count;i++)
        nccr_walk_Attribute(node->atts.values[i],nodes,map);
    for(i=0;i<node->groups.count;i++)
        nccr_walk_Group(node->groups.values[i],nodes,map);
    for(i=0;i<node->enumTypes.count;i++)
        nccr_walk_EnumTypedef(node->enumTypes.values[i],nodes,map);
}

static int
nccr_walk_Dimension(Dimension* node, NClist* nodes, NChashmap* map)
{
    crnote((Notes*)node,nodes,map);
}

static int
nccr_walk_Variable(Variable* node, NClist* nodes, NChashmap* map)
{
    int i;
    crnote((Notes*)node,nodes,map);
    for(i=0;i<node->shape.count;i++)
        nccr_walk_Dimension(node->shape.values[i],nodes,map);
    for(i=0;i<node->atts.count;i++)
        nccr_walk_Attribute(node->atts.values[i],nodes,map);
}

static int
nccr_walk_Structure(Structure* node, NClist* nodes, NChashmap* map)
{
    int i;
    crnote((Notes*)node,nodes,map);
    for(i=0;i<node->shape.count;i++)
        nccr_walk_Dimension(node->shape.values[i],nodes,map);
    for(i=0;i<node->atts.count;i++)
        nccr_walk_Attribute(node->atts.values[i],nodes,map);
    for(i=0;i<node->vars.count;i++)
        nccr_walk_Variable(node->vars.values[i],nodes,map);
    for(i=0;i<node->structs.count;i++)
        nccr_walk_Structure(node->structs.values[i],nodes,map);
}

static int
nccr_walk_Attribute(Attribute* node, NClist* nodes, NChashmap* map)
{
    crnote((Notes*)node,nodes,map);
}

static int
nccr_walk_EnumTypedef(EnumTypedef* node, NClist* nodes, NChashmap* map)
{
    int i;
    crnote((Notes*)node,nodes,map);
    for(i=0;i<node->map.count;i++)
        nccr_walk_EnumType(node->map.values[i],nodes,map);

}

static int
nccr_walk_EnumType(EnumType* node, NClist* nodes, NChashmap* map)
{
    crnote((Notes*)node,nodes,map);
}



