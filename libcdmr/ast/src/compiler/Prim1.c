#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "Prim1.h"

int
PrimRequired_write(ast_runtime* rt, PrimRequired* primrequired_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write_primitive(rt,ast_int32,1,&primrequired_v->f_int32);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_int64,2,&primrequired_v->f_int64);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_uint32,3,&primrequired_v->f_uint32);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_uint64,4,&primrequired_v->f_uint64);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_sint32,5,&primrequired_v->f_sint32);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_sint64,6,&primrequired_v->f_sint64);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_fixed32,7,&primrequired_v->f_fixed32);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_fixed64,8,&primrequired_v->f_fixed64);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_sfixed32,9,&primrequired_v->f_sfixed32);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_sfixed64,10,&primrequired_v->f_sfixed64);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_double,11,&primrequired_v->f_double);
    if(status != AST_NOERR) {goto done;}
    status = ast_write_primitive(rt,ast_float,12,&primrequired_v->f_float);
    if(status != AST_NOERR) {goto done;}

done:
    return status;

} /*PrimRequired_write*/

int
PrimRequired_read(ast_runtime* rt, PrimRequired** primrequired_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    PrimRequired* primrequired_v;

    primrequired_v = (PrimRequired*)ast_alloc(rt,sizeof(PrimRequired));
    if(primrequired_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&primrequired_v->f_int32);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int64,2,&primrequired_v->f_int64);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint32,3,&primrequired_v->f_uint32);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_uint64,4,&primrequired_v->f_uint64);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_sint32,5,&primrequired_v->f_sint32);
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_sint64,6,&primrequired_v->f_sint64);
            } break;
        case 7: {
            status = ast_read_primitive(rt,ast_fixed32,7,&primrequired_v->f_fixed32);
            } break;
        case 8: {
            status = ast_read_primitive(rt,ast_fixed64,8,&primrequired_v->f_fixed64);
            } break;
        case 9: {
            status = ast_read_primitive(rt,ast_sfixed32,9,&primrequired_v->f_sfixed32);
            } break;
        case 10: {
            status = ast_read_primitive(rt,ast_sfixed64,10,&primrequired_v->f_sfixed64);
            } break;
        case 11: {
            status = ast_read_primitive(rt,ast_double,11,&primrequired_v->f_double);
            } break;
        case 12: {
            status = ast_read_primitive(rt,ast_float,12,&primrequired_v->f_float);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
    if(primrequired_vp) *primrequired_vp = primrequired_v;
done:
    return status;
} /*PrimRequired_read*/

int
PrimRequired_reclaim(ast_runtime* rt, PrimRequired* primrequired_v)
{
    int status = AST_NOERR;
    int i = 0;

    ast_free(rt,(void*)primrequired_v);

done:
    return status;

} /*PrimRequired_reclaim*/

long
PrimRequired_size(ast_runtime* rt, PrimRequired* primrequired_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_int32,&primrequired_v->f_int32);
    totalsize += ast_write_size(rt,ast_int64,&primrequired_v->f_int64);
    totalsize += ast_write_size(rt,ast_uint32,&primrequired_v->f_uint32);
    totalsize += ast_write_size(rt,ast_uint64,&primrequired_v->f_uint64);
    totalsize += ast_write_size(rt,ast_sint32,&primrequired_v->f_sint32);
    totalsize += ast_write_size(rt,ast_sint64,&primrequired_v->f_sint64);
    totalsize += ast_write_size(rt,ast_fixed32,&primrequired_v->f_fixed32);
    totalsize += ast_write_size(rt,ast_fixed64,&primrequired_v->f_fixed64);
    totalsize += ast_write_size(rt,ast_sfixed32,&primrequired_v->f_sfixed32);
    totalsize += ast_write_size(rt,ast_sfixed64,&primrequired_v->f_sfixed64);
    totalsize += ast_write_size(rt,ast_double,&primrequired_v->f_double);
    totalsize += ast_write_size(rt,ast_float,&primrequired_v->f_float);
    return status;

} /*PrimRequired_write_size*/
int
PrimOptional_write(ast_runtime* rt, PrimOptional* primoptional_v)
{
    int status = AST_NOERR;
    int i = 0;

    if(primoptional_v->f_int32.exists) {
        status = ast_write_primitive(rt,ast_int32,1,&primoptional_v->f_int32.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_int64.exists) {
        status = ast_write_primitive(rt,ast_int64,2,&primoptional_v->f_int64.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_uint32.exists) {
        status = ast_write_primitive(rt,ast_uint32,3,&primoptional_v->f_uint32.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_uint64.exists) {
        status = ast_write_primitive(rt,ast_uint64,4,&primoptional_v->f_uint64.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_sint32.exists) {
        status = ast_write_primitive(rt,ast_sint32,5,&primoptional_v->f_sint32.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_sint64.exists) {
        status = ast_write_primitive(rt,ast_sint64,6,&primoptional_v->f_sint64.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_fixed32.exists) {
        status = ast_write_primitive(rt,ast_fixed32,7,&primoptional_v->f_fixed32.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_fixed64.exists) {
        status = ast_write_primitive(rt,ast_fixed64,8,&primoptional_v->f_fixed64.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_sfixed32.exists) {
        status = ast_write_primitive(rt,ast_sfixed32,9,&primoptional_v->f_sfixed32.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_sfixed64.exists) {
        status = ast_write_primitive(rt,ast_sfixed64,10,&primoptional_v->f_sfixed64.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_double.exists) {
        status = ast_write_primitive(rt,ast_double,11,&primoptional_v->f_double.value);
        if(status != AST_NOERR) {goto done;}
    }
    if(primoptional_v->f_float.exists) {
        status = ast_write_primitive(rt,ast_float,12,&primoptional_v->f_float.value);
        if(status != AST_NOERR) {goto done;}
    }

done:
    return status;

} /*PrimOptional_write*/

int
PrimOptional_read(ast_runtime* rt, PrimOptional** primoptional_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    PrimOptional* primoptional_v;

    primoptional_v = (PrimOptional*)ast_alloc(rt,sizeof(PrimOptional));
    if(primoptional_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            primoptional_v->f_int32.exists = 1;
            primoptional_v->f_int32.value = 0;
            status = ast_read_primitive(rt,ast_int32,1,&primoptional_v->f_int32.value);
            } break;
        case 2: {
            primoptional_v->f_int64.exists = 1;
            primoptional_v->f_int64.value = 0;
            status = ast_read_primitive(rt,ast_int64,2,&primoptional_v->f_int64.value);
            } break;
        case 3: {
            primoptional_v->f_uint32.exists = 1;
            primoptional_v->f_uint32.value = 0;
            status = ast_read_primitive(rt,ast_uint32,3,&primoptional_v->f_uint32.value);
            } break;
        case 4: {
            primoptional_v->f_uint64.exists = 1;
            primoptional_v->f_uint64.value = 0;
            status = ast_read_primitive(rt,ast_uint64,4,&primoptional_v->f_uint64.value);
            } break;
        case 5: {
            primoptional_v->f_sint32.exists = 1;
            primoptional_v->f_sint32.value = 0;
            status = ast_read_primitive(rt,ast_sint32,5,&primoptional_v->f_sint32.value);
            } break;
        case 6: {
            primoptional_v->f_sint64.exists = 1;
            primoptional_v->f_sint64.value = 0;
            status = ast_read_primitive(rt,ast_sint64,6,&primoptional_v->f_sint64.value);
            } break;
        case 7: {
            primoptional_v->f_fixed32.exists = 1;
            primoptional_v->f_fixed32.value = 0;
            status = ast_read_primitive(rt,ast_fixed32,7,&primoptional_v->f_fixed32.value);
            } break;
        case 8: {
            primoptional_v->f_fixed64.exists = 1;
            primoptional_v->f_fixed64.value = 0;
            status = ast_read_primitive(rt,ast_fixed64,8,&primoptional_v->f_fixed64.value);
            } break;
        case 9: {
            primoptional_v->f_sfixed32.exists = 1;
            primoptional_v->f_sfixed32.value = 0;
            status = ast_read_primitive(rt,ast_sfixed32,9,&primoptional_v->f_sfixed32.value);
            } break;
        case 10: {
            primoptional_v->f_sfixed64.exists = 1;
            primoptional_v->f_sfixed64.value = 0;
            status = ast_read_primitive(rt,ast_sfixed64,10,&primoptional_v->f_sfixed64.value);
            } break;
        case 11: {
            primoptional_v->f_double.exists = 1;
            primoptional_v->f_double.value = 0;
            status = ast_read_primitive(rt,ast_double,11,&primoptional_v->f_double.value);
            } break;
        case 12: {
            primoptional_v->f_float.exists = 1;
            primoptional_v->f_float.value = 0;
            status = ast_read_primitive(rt,ast_float,12,&primoptional_v->f_float.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
    if(primoptional_v->f_int32.exists) {
        primoptional_v->f_int32.value = 1;
    }
    if(primoptional_v->f_int64.exists) {
        primoptional_v->f_int64.value = 2;
    }
    if(primoptional_v->f_uint32.exists) {
        primoptional_v->f_uint32.value = 3;
    }
    if(primoptional_v->f_uint64.exists) {
        primoptional_v->f_uint64.value = 4;
    }
    if(primoptional_v->f_sint32.exists) {
        primoptional_v->f_sint32.value = 5;
    }
    if(primoptional_v->f_sint64.exists) {
        primoptional_v->f_sint64.value = 6;
    }
    if(primoptional_v->f_fixed32.exists) {
        primoptional_v->f_fixed32.value = 7;
    }
    if(primoptional_v->f_fixed64.exists) {
        primoptional_v->f_fixed64.value = 8;
    }
    if(primoptional_v->f_sfixed32.exists) {
        primoptional_v->f_sfixed32.value = 9;
    }
    if(primoptional_v->f_sfixed64.exists) {
        primoptional_v->f_sfixed64.value = 10;
    }
    if(primoptional_v->f_double.exists) {
        primoptional_v->f_double.value = 11;
    }
    if(primoptional_v->f_float.exists) {
        primoptional_v->f_float.value = 12;
    }
    if(primoptional_vp) *primoptional_vp = primoptional_v;
done:
    return status;
} /*PrimOptional_read*/

int
PrimOptional_reclaim(ast_runtime* rt, PrimOptional* primoptional_v)
{
    int status = AST_NOERR;
    int i = 0;

    ast_free(rt,(void*)primoptional_v);

done:
    return status;

} /*PrimOptional_reclaim*/

long
PrimOptional_size(ast_runtime* rt, PrimOptional* primoptional_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    if(primoptional_v->f_int32.exists) {
        totalsize += ast_write_size(rt,ast_int32,&primoptional_v->f_int32.value);
    }
    if(primoptional_v->f_int64.exists) {
        totalsize += ast_write_size(rt,ast_int64,&primoptional_v->f_int64.value);
    }
    if(primoptional_v->f_uint32.exists) {
        totalsize += ast_write_size(rt,ast_uint32,&primoptional_v->f_uint32.value);
    }
    if(primoptional_v->f_uint64.exists) {
        totalsize += ast_write_size(rt,ast_uint64,&primoptional_v->f_uint64.value);
    }
    if(primoptional_v->f_sint32.exists) {
        totalsize += ast_write_size(rt,ast_sint32,&primoptional_v->f_sint32.value);
    }
    if(primoptional_v->f_sint64.exists) {
        totalsize += ast_write_size(rt,ast_sint64,&primoptional_v->f_sint64.value);
    }
    if(primoptional_v->f_fixed32.exists) {
        totalsize += ast_write_size(rt,ast_fixed32,&primoptional_v->f_fixed32.value);
    }
    if(primoptional_v->f_fixed64.exists) {
        totalsize += ast_write_size(rt,ast_fixed64,&primoptional_v->f_fixed64.value);
    }
    if(primoptional_v->f_sfixed32.exists) {
        totalsize += ast_write_size(rt,ast_sfixed32,&primoptional_v->f_sfixed32.value);
    }
    if(primoptional_v->f_sfixed64.exists) {
        totalsize += ast_write_size(rt,ast_sfixed64,&primoptional_v->f_sfixed64.value);
    }
    if(primoptional_v->f_double.exists) {
        totalsize += ast_write_size(rt,ast_double,&primoptional_v->f_double.value);
    }
    if(primoptional_v->f_float.exists) {
        totalsize += ast_write_size(rt,ast_float,&primoptional_v->f_float.value);
    }
    return status;

} /*PrimOptional_write_size*/
int
PrimRepeated_write(ast_runtime* rt, PrimRepeated* primrepeated_v)
{
    int status = AST_NOERR;
    int i = 0;

    for(i=0;i<primrepeated_v->f_int32.count;i++) {
        status = ast_write_primitive(rt,ast_int32,1,&primrepeated_v->f_int32.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_int64.count;i++) {
        status = ast_write_primitive(rt,ast_int64,2,&primrepeated_v->f_int64.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_uint32.count;i++) {
        status = ast_write_primitive(rt,ast_uint32,3,&primrepeated_v->f_uint32.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_uint64.count;i++) {
        status = ast_write_primitive(rt,ast_uint64,4,&primrepeated_v->f_uint64.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_sint32.count;i++) {
        status = ast_write_primitive(rt,ast_sint32,5,&primrepeated_v->f_sint32.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_sint64.count;i++) {
        status = ast_write_primitive(rt,ast_sint64,6,&primrepeated_v->f_sint64.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_fixed32.count;i++) {
        status = ast_write_primitive(rt,ast_fixed32,7,&primrepeated_v->f_fixed32.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_fixed64.count;i++) {
        status = ast_write_primitive(rt,ast_fixed64,8,&primrepeated_v->f_fixed64.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_sfixed32.count;i++) {
        status = ast_write_primitive(rt,ast_sfixed32,9,&primrepeated_v->f_sfixed32.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_sfixed64.count;i++) {
        status = ast_write_primitive(rt,ast_sfixed64,10,&primrepeated_v->f_sfixed64.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_double.count;i++) {
        status = ast_write_primitive(rt,ast_double,11,&primrepeated_v->f_double.values[i]);
        if(status != AST_NOERR) {goto done;}
    }
    for(i=0;i<primrepeated_v->f_float.count;i++) {
        status = ast_write_primitive(rt,ast_float,12,&primrepeated_v->f_float.values[i]);
        if(status != AST_NOERR) {goto done;}
    }

done:
    return status;

} /*PrimRepeated_write*/

int
PrimRepeated_read(ast_runtime* rt, PrimRepeated** primrepeated_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    PrimRepeated* primrepeated_v;

    primrepeated_v = (PrimRepeated*)ast_alloc(rt,sizeof(PrimRepeated));
    if(primrepeated_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            int i;

            size_t count;

            primrepeated_v->f_int32.count = 0;
            primrepeated_v->f_int32.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_int32.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_int32.count;i++) {
                int32_t tmp;
                status = ast_read_primitive(rt,ast_int32,1,&tmp);
                status = ast_repeat_append(rt,ast_int32,&primrepeated_v->f_int32,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 2: {
            int i;

            size_t count;

            primrepeated_v->f_int64.count = 0;
            primrepeated_v->f_int64.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_int64.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_int64.count;i++) {
                int64_t tmp;
                status = ast_read_primitive(rt,ast_int64,2,&tmp);
                status = ast_repeat_append(rt,ast_int64,&primrepeated_v->f_int64,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 3: {
            int i;

            size_t count;

            primrepeated_v->f_uint32.count = 0;
            primrepeated_v->f_uint32.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_uint32.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_uint32.count;i++) {
                uint32_t tmp;
                status = ast_read_primitive(rt,ast_uint32,3,&tmp);
                status = ast_repeat_append(rt,ast_uint32,&primrepeated_v->f_uint32,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 4: {
            int i;

            size_t count;

            primrepeated_v->f_uint64.count = 0;
            primrepeated_v->f_uint64.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_uint64.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_uint64.count;i++) {
                uint64_t tmp;
                status = ast_read_primitive(rt,ast_uint64,4,&tmp);
                status = ast_repeat_append(rt,ast_uint64,&primrepeated_v->f_uint64,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 5: {
            int i;

            size_t count;

            primrepeated_v->f_sint32.count = 0;
            primrepeated_v->f_sint32.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_sint32.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_sint32.count;i++) {
                int32_t tmp;
                status = ast_read_primitive(rt,ast_sint32,5,&tmp);
                status = ast_repeat_append(rt,ast_sint32,&primrepeated_v->f_sint32,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 6: {
            int i;

            size_t count;

            primrepeated_v->f_sint64.count = 0;
            primrepeated_v->f_sint64.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_sint64.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_sint64.count;i++) {
                int64_t tmp;
                status = ast_read_primitive(rt,ast_sint64,6,&tmp);
                status = ast_repeat_append(rt,ast_sint64,&primrepeated_v->f_sint64,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 7: {
            int i;

            size_t count;

            primrepeated_v->f_fixed32.count = 0;
            primrepeated_v->f_fixed32.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_fixed32.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_fixed32.count;i++) {
                uint32_t tmp;
                status = ast_read_primitive(rt,ast_fixed32,7,&tmp);
                status = ast_repeat_append(rt,ast_fixed32,&primrepeated_v->f_fixed32,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 8: {
            int i;

            size_t count;

            primrepeated_v->f_fixed64.count = 0;
            primrepeated_v->f_fixed64.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_fixed64.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_fixed64.count;i++) {
                uint64_t tmp;
                status = ast_read_primitive(rt,ast_fixed64,8,&tmp);
                status = ast_repeat_append(rt,ast_fixed64,&primrepeated_v->f_fixed64,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 9: {
            int i;

            size_t count;

            primrepeated_v->f_sfixed32.count = 0;
            primrepeated_v->f_sfixed32.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_sfixed32.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_sfixed32.count;i++) {
                int32_t tmp;
                status = ast_read_primitive(rt,ast_sfixed32,9,&tmp);
                status = ast_repeat_append(rt,ast_sfixed32,&primrepeated_v->f_sfixed32,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 10: {
            int i;

            size_t count;

            primrepeated_v->f_sfixed64.count = 0;
            primrepeated_v->f_sfixed64.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_sfixed64.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_sfixed64.count;i++) {
                int64_t tmp;
                status = ast_read_primitive(rt,ast_sfixed64,10,&tmp);
                status = ast_repeat_append(rt,ast_sfixed64,&primrepeated_v->f_sfixed64,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 11: {
            int i;

            size_t count;

            primrepeated_v->f_double.count = 0;
            primrepeated_v->f_double.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_double.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_double.count;i++) {
                double tmp;
                status = ast_read_primitive(rt,ast_double,11,&tmp);
                status = ast_repeat_append(rt,ast_double,&primrepeated_v->f_double,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        case 12: {
            int i;

            size_t count;

            primrepeated_v->f_float.count = 0;
            primrepeated_v->f_float.values = NULL;
            status = ast_read_count(rt,&primrepeated_v->f_float.count);
            if(status != AST_NOERR) {goto done;}
            for(i=0;i<primrepeated_v->f_float.count;i++) {
                float tmp;
                status = ast_read_primitive(rt,ast_float,12,&tmp);
                status = ast_repeat_append(rt,ast_float,&primrepeated_v->f_float,&tmp);
                if(status != AST_NOERR) {goto done;}
            } /*for*/;
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
    if(primrepeated_vp) *primrepeated_vp = primrepeated_v;
done:
    return status;
} /*PrimRepeated_read*/

int
PrimRepeated_reclaim(ast_runtime* rt, PrimRepeated* primrepeated_v)
{
    int status = AST_NOERR;
    int i = 0;

    ast_free(rt,(void*)primrepeated_v);

done:
    return status;

} /*PrimRepeated_reclaim*/

long
PrimRepeated_size(ast_runtime* rt, PrimRepeated* primrepeated_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    for(i=0;i<primrepeated_v->f_int32.count;i++) {
        totalsize += ast_write_size(rt,ast_int32,primrepeated_v->f_int32.values[i]);
    }
    for(i=0;i<primrepeated_v->f_int64.count;i++) {
        totalsize += ast_write_size(rt,ast_int64,primrepeated_v->f_int64.values[i]);
    }
    for(i=0;i<primrepeated_v->f_uint32.count;i++) {
        totalsize += ast_write_size(rt,ast_uint32,primrepeated_v->f_uint32.values[i]);
    }
    for(i=0;i<primrepeated_v->f_uint64.count;i++) {
        totalsize += ast_write_size(rt,ast_uint64,primrepeated_v->f_uint64.values[i]);
    }
    for(i=0;i<primrepeated_v->f_sint32.count;i++) {
        totalsize += ast_write_size(rt,ast_sint32,primrepeated_v->f_sint32.values[i]);
    }
    for(i=0;i<primrepeated_v->f_sint64.count;i++) {
        totalsize += ast_write_size(rt,ast_sint64,primrepeated_v->f_sint64.values[i]);
    }
    for(i=0;i<primrepeated_v->f_fixed32.count;i++) {
        totalsize += ast_write_size(rt,ast_fixed32,primrepeated_v->f_fixed32.values[i]);
    }
    for(i=0;i<primrepeated_v->f_fixed64.count;i++) {
        totalsize += ast_write_size(rt,ast_fixed64,primrepeated_v->f_fixed64.values[i]);
    }
    for(i=0;i<primrepeated_v->f_sfixed32.count;i++) {
        totalsize += ast_write_size(rt,ast_sfixed32,primrepeated_v->f_sfixed32.values[i]);
    }
    for(i=0;i<primrepeated_v->f_sfixed64.count;i++) {
        totalsize += ast_write_size(rt,ast_sfixed64,primrepeated_v->f_sfixed64.values[i]);
    }
    for(i=0;i<primrepeated_v->f_double.count;i++) {
        totalsize += ast_write_size(rt,ast_double,primrepeated_v->f_double.values[i]);
    }
    for(i=0;i<primrepeated_v->f_float.count;i++) {
        totalsize += ast_write_size(rt,ast_float,primrepeated_v->f_float.values[i]);
    }
    return status;

} /*PrimRepeated_write_size*/
