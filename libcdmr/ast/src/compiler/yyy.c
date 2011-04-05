#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "yyy.h"

ast_err
Annotation_write(ast_runtime* rt, Annotation* annotation_v)
{
    ast_err status = AST_NOERR;


done:
    return status;

} /*Annotation_write*/

ast_err
Annotation_read(ast_runtime* rt, Annotation** annotation_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Annotation* annotation_v;

    annotation_v = (Annotation*)ast_alloc(rt,sizeof(Annotation));
    if(annotation_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {goto done;}
    if(annotation_vp) *annotation_vp = annotation_v;
done:
    return status;
} /*Annotation_read*/

ast_err
Annotation_reclaim(ast_runtime* rt, Annotation* annotation_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)annotation_v);
    goto done;

done:
    return status;

} /*Annotation_reclaim*/

size_t
Annotation_get_size(ast_runtime* rt, Annotation* annotation_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    return totalsize;

} /*Annotation_get_size*/

ast_err
m1_write(ast_runtime* rt, m1* m1_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&m1_v->f1);
        if(status != AST_NOERR) {goto done;}
    }
    {
        size_t size;
        if(m1_v->f2.defined) {
            status = ast_write_tag(rt,ast_counted,2);
            if(status != AST_NOERR) {goto done;}
            size = Annotation_get_size(rt,m1_v->f2.value);
            status = ast_write_count(rt,size);
            if(status != AST_NOERR) {goto done;}
            status = Annotation_write(rt,m1_v->f2.value);
            if(status != AST_NOERR) {goto done;}
        }
    }

done:
    return status;

} /*m1_write*/

ast_err
m1_read(ast_runtime* rt, m1** m1_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    m1* m1_v;

    m1_v = (m1*)ast_alloc(rt,sizeof(m1));
    if(m1_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&m1_v->f1);
            } break;
        case 2: {
            size_t count;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_count(rt,&count);
            if(status != AST_NOERR) {goto done;}
            status = ast_mark(rt,count);
            if(status != AST_NOERR) {goto done;}
            m1_v->f2.defined = 1;
            m1_v->f2.value = NULL;
            status = Annotation_read(rt,&m1_v->f2.value);
            if(status != AST_NOERR) {goto done;}
            status = ast_unmark(rt);
            if(status != AST_NOERR) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {goto done;}
    if(m1_vp) *m1_vp = m1_v;
done:
    return status;
} /*m1_read*/

ast_err
m1_reclaim(ast_runtime* rt, m1* m1_v)
{
    ast_err status = AST_NOERR;

    {
        if(m1_v->f2.defined) {
            status = Annotation_reclaim(rt,m1_v->f2.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    ast_free(rt,(void*)m1_v);
    goto done;

done:
    return status;

} /*m1_reclaim*/

size_t
m1_get_size(ast_runtime* rt, m1* m1_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&m1_v->f1);
        totalsize += fieldsize;
    }
    {
        if(m1_v->f2.defined) {
            fieldsize += Annotation_get_size(rt,m1_v->f2.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*m1_get_size*/

