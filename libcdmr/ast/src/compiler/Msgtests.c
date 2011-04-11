#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "Msgtests.h"

ast_err
Msgtest_write(ast_runtime* rt, Msgtest* msgtest_v)
{
    ast_err status = AST_NOERR;

    {
        size_t size;
        status = ast_write_tag(rt,ast_counted,1);
        if(status != AST_NOERR) {goto done;}
        size = Submsg_get_size(rt,msgtest_v->rmsg);
        status = ast_write_count(rt,size);
        if(status != AST_NOERR) {goto done;}
        status = Submsg_write(rt,msgtest_v->rmsg);
        if(status != AST_NOERR) {goto done;}
    }
    {
        size_t size;
        if(msgtest_v->omsg.defined) {
            status = ast_write_tag(rt,ast_counted,2);
            if(status != AST_NOERR) {goto done;}
            size = Submsg_get_size(rt,msgtest_v->omsg.value);
            status = ast_write_count(rt,size);
            if(status != AST_NOERR) {goto done;}
            status = Submsg_write(rt,msgtest_v->omsg.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<msgtest_v->pmsg.count;i++) {
            status = ast_write_tag(rt,ast_counted,3);
            if(status != AST_NOERR) {goto done;}
            size = Submsg_get_size(rt,msgtest_v->pmsg.values[i]);
            status = ast_write_count(rt,size);
            if(status != AST_NOERR) {goto done;}
            status = Submsg_write(rt,msgtest_v->pmsg.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }

done:
    return status;

} /*Msgtest_write*/

ast_err
Msgtest_read(ast_runtime* rt, Msgtest** msgtest_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Msgtest* msgtest_v;

    msgtest_v = (Msgtest*)ast_alloc(rt,sizeof(Msgtest));
    if(msgtest_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            size_t count;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_count(rt,&count);
            if(status != AST_NOERR) {goto done;}
            status = ast_mark(rt,count);
            if(status != AST_NOERR) {goto done;}
            status = Submsg_read(rt,&msgtest_v->rmsg);
            if(status != AST_NOERR) {goto done;}
            status = ast_unmark(rt);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 2: {
            size_t count;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_count(rt,&count);
            if(status != AST_NOERR) {goto done;}
            status = ast_mark(rt,count);
            if(status != AST_NOERR) {goto done;}
            msgtest_v->omsg.defined = 1;
            msgtest_v->omsg.value = NULL;
            status = Submsg_read(rt,&msgtest_v->omsg.value);
            if(status != AST_NOERR) {goto done;}
            status = ast_unmark(rt);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 3: {
            size_t count;
            Submsg* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_count(rt,&count);
            if(status != AST_NOERR) {goto done;}
            status = ast_mark(rt,count);
            if(status != AST_NOERR) {goto done;}
            status = Submsg_read(rt,&tmp);
            status = ast_repeat_append(rt,ast_message,&msgtest_v->pmsg,&tmp);
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
    if(msgtest_vp) *msgtest_vp = msgtest_v;
done:
    return status;
} /*Msgtest_read*/

ast_err
Msgtest_reclaim(ast_runtime* rt, Msgtest* msgtest_v)
{
    ast_err status = AST_NOERR;

    {
        status = Submsg_reclaim(rt,msgtest_v->rmsg);
        if(status != AST_NOERR) {goto done;}
    }
    {
        if(msgtest_v->omsg.defined) {
            status = Submsg_reclaim(rt,msgtest_v->omsg.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i;
        for(i=0;i<msgtest_v->pmsg.count;i++) {
            status = Submsg_reclaim(rt,msgtest_v->pmsg.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
        ast_free(rt,msgtest_v->pmsg.values);
    }
    ast_free(rt,(void*)msgtest_v);
    goto done;

done:
    return status;

} /*Msgtest_reclaim*/

size_t
Msgtest_get_size(ast_runtime* rt, Msgtest* msgtest_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += Submsg_get_size(rt,msgtest_v->rmsg);
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
        totalsize += fieldsize;
    }
    {
        if(msgtest_v->omsg.defined) {
            fieldsize += Submsg_get_size(rt,msgtest_v->omsg.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<msgtest_v->pmsg.count;i++) {
            fieldsize += Submsg_get_size(rt,msgtest_v->pmsg.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Msgtest_get_size*/

ast_err
Submsg_write(ast_runtime* rt, Submsg* submsg_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&submsg_v->f_int32);
        if(status != AST_NOERR) {goto done;}
    }

done:
    return status;

} /*Submsg_write*/

ast_err
Submsg_read(ast_runtime* rt, Submsg** submsg_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Submsg* submsg_v;

    submsg_v = (Submsg*)ast_alloc(rt,sizeof(Submsg));
    if(submsg_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&submsg_v->f_int32);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {goto done;}
    if(submsg_vp) *submsg_vp = submsg_v;
done:
    return status;
} /*Submsg_read*/

ast_err
Submsg_reclaim(ast_runtime* rt, Submsg* submsg_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)submsg_v);
    goto done;

done:
    return status;

} /*Submsg_reclaim*/

size_t
Submsg_get_size(ast_runtime* rt, Submsg* submsg_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&submsg_v->f_int32);
        totalsize += fieldsize;
    }
    return totalsize;

} /*Submsg_get_size*/

