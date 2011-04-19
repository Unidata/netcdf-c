
#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "t2.h"
#include "t.h"

int
t_write(ast_runtime* rt, t* t_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_int32,1,&t_v->f1_f);
    if(!status) {goto done;}
    if(t_v->f2_f.exists) {
        status = ast_write(rt,ast_float,2,&t_v->f2_f.value);
        if(!status) {goto done;}
    }
    for(i=0;i<t_v->f3_f.count;i++) {
        status = ast_write(rt,ast_sint32,3,&t_v->f3_f.values[i]);
        if(!status) {goto done;}
    }
    status = m2_write(rt,t_v->f4_f);
    if(!status) {goto done;}

done:
    return status;

} /*t_write*/

int
t_read(ast_runtime* rt, t** t_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    int i = 0;
    size_t count;
    t* t_v;

    t_v = (t*)ast_alloc(sizeof(t));
    if(t_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_int32,&t_v->f1_f);
            } break;
        case 2: {
            t_v->f2_f.exists = 1;
            t_v->f2_f.value = 0;
            status = ast_read(rt,ast_float,&t_v->f2_f.value);
            } break;
        case 3: {
            t_v->f3_f.count = 0;
            t_v->f3_f.values = NULL;
            status = ast_read_count(rt,&t_v->f3_f.count);
            if(!status) {goto done;}
            for(i=0;i<t_v->f3_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_sint32,&tmp);
                status = ast_append(rt,ast_sint32,&t_v->f3_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 4: {
            status = m2_read(rt,&t_v->f4_f);
            if(!status) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(t_v->f1_f.exists) {
    t_v->f1_f.value = 5;
}
if(t_v->f2_f.exists) {
    t_v->f2_f.value = 0;
}
if(t_v->f3_f.exists) {
    t_v->f3_f.value = 0;
}
done:
    return status;
} /*t_read*/

int
t_reclaim(ast_runtime* rt, t* t_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = m2_reclaim(rt,t_v->f4_f);
    if(!status) {goto done;}

done:
    return status;

} /*t_reclaim*/

long
t_size(ast_runtime* rt, t* t_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_int32,&t_v->f1_f);
    if(t_v->f2_f.exists) {
        totalsize += ast_write_size(rt,ast_float,&t_v->f2_f.value);
    }
    for(i=0;i<t_v->f3_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sint32,t_v->f3_f.values[i]);
    }
    totalsize += m2_write_size(rt,4,t_v->f4_f);
    return status;

} /*t_write_size*/
