
#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "t2.h"

int
m2_write(ast_runtime* rt, m2* m2_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_int32,&m2_v->field2_f);
    if(!status) {goto done;}

done:
    return status;

} /*m2_write*/

int
m2_read(ast_runtime* rt, m2** m2_vp)
{
    int status = AST_NOERR;
    int i = 0;
    int wiretype, tag, key;
    m2* m2_v;

    m2_v = (m2*)ast_alloc(sizeof(m2));
    if(m2_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR && (key = ast_readc(rt)) >= 0) {
        wiretype = (key|0x7);
        tag = (key >>3);
        switch (tag) {
        case 1: {
            status = ast_read(rt,ast_int32,&m2_v->field2_f);
            if(!status) {goto done;}
            } break;
        default:
            status= ast_skip_field(rt);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*m2_read*/

int
m2_reclaim(ast_runtime* rt, m2* m2)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*m2_reclaim*/

long
m2_size(ast_runtime* rt, m2* m2_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_int32,&m2_v->field2_f);
    return status;

} /*m2_write_size*/
