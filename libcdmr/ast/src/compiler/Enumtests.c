#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "Enumtests.h"

ast_err
Enumtest_write(ast_runtime* rt, Enumtest* enumtest_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_enum,1,&enumtest_v->renum);
        if(status != AST_NOERR) {goto done;}
    }
    {
        if(enumtest_v->oenum.defined) {
            status = ast_write_primitive(rt,ast_enum,2,&enumtest_v->oenum.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<enumtest_v->penum.count;i++) {
            status = ast_write_primitive(rt,ast_enum,3,&enumtest_v->penum.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<enumtest_v->ppenum.count;i++) {
            status = ast_write_primitive(rt,ast_enum,4,&enumtest_v->ppenum.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }

done:
    return status;

} /*Enumtest_write*/

ast_err
Enumtest_read(ast_runtime* rt, Enumtest** enumtest_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Enumtest* enumtest_v;

    enumtest_v = (Enumtest*)ast_alloc(rt,sizeof(Enumtest));
    if(enumtest_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_enum,1,&enumtest_v->renum);
            } break;
        case 2: {
            enumtest_v->oenum.defined = 1;
            enumtest_v->oenum.value = 0;
            status = ast_read_primitive(rt,ast_enum,2,&enumtest_v->oenum.value);
            } break;
        case 3: {
            Testenum tmp;
            status = ast_read_primitive(rt,ast_enum,3,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_enum,&enumtest_v->penum,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 4: {
            Testenum tmp;
            status = ast_read_primitive(rt,ast_enum,4,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_enum,&enumtest_v->ppenum,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {goto done;}
    if(enumtest_vp) *enumtest_vp = enumtest_v;
done:
    return status;
} /*Enumtest_read*/

ast_err
Enumtest_reclaim(ast_runtime* rt, Enumtest* enumtest_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)enumtest_v);
    goto done;

done:
    return status;

} /*Enumtest_reclaim*/

size_t
Enumtest_get_size(ast_runtime* rt, Enumtest* enumtest_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_enum,&enumtest_v->renum);
        totalsize += fieldsize;
    }
    {
        if(enumtest_v->oenum.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
            fieldsize += ast_get_size(rt,ast_enum,&enumtest_v->oenum.value);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<enumtest_v->penum.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_enum,&enumtest_v->penum.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<enumtest_v->ppenum.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_enum,&enumtest_v->ppenum.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Enumtest_get_size*/

