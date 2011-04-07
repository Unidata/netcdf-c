#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "Primtests.h"

ast_err
PrimRepeated_write(ast_runtime* rt, PrimRepeated* primrepeated_v)
{
    ast_err status = AST_NOERR;

    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_int32.count;i++) {
            status = ast_write_primitive(rt,ast_int32,1,&primrepeated_v->f_int32.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_int64.count;i++) {
            status = ast_write_primitive(rt,ast_int64,2,&primrepeated_v->f_int64.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_uint32.count;i++) {
            status = ast_write_primitive(rt,ast_uint32,3,&primrepeated_v->f_uint32.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_uint64.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&primrepeated_v->f_uint64.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_sint32.count;i++) {
            status = ast_write_primitive(rt,ast_sint32,5,&primrepeated_v->f_sint32.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_sint64.count;i++) {
            status = ast_write_primitive(rt,ast_sint64,6,&primrepeated_v->f_sint64.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_fixed32.count;i++) {
            status = ast_write_primitive(rt,ast_fixed32,7,&primrepeated_v->f_fixed32.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_fixed64.count;i++) {
            status = ast_write_primitive(rt,ast_fixed64,8,&primrepeated_v->f_fixed64.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_sfixed32.count;i++) {
            status = ast_write_primitive(rt,ast_sfixed32,9,&primrepeated_v->f_sfixed32.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_sfixed64.count;i++) {
            status = ast_write_primitive(rt,ast_sfixed64,10,&primrepeated_v->f_sfixed64.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_double.count;i++) {
            status = ast_write_primitive(rt,ast_double,11,&primrepeated_v->f_double.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<primrepeated_v->f_float.count;i++) {
            status = ast_write_primitive(rt,ast_float,12,&primrepeated_v->f_float.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }

done:
    return status;

} /*PrimRepeated_write*/

ast_err
PrimRepeated_read(ast_runtime* rt, PrimRepeated** primrepeated_vp)
{
    ast_err status = AST_NOERR;
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
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,1,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_int32,&primrepeated_v->f_int32,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 2: {
            int64_t tmp;
            status = ast_read_primitive(rt,ast_int64,2,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_int64,&primrepeated_v->f_int64,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 3: {
            uint32_t tmp;
            status = ast_read_primitive(rt,ast_uint32,3,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_uint32,&primrepeated_v->f_uint32,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_uint64,&primrepeated_v->f_uint64,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 5: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_sint32,5,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_sint32,&primrepeated_v->f_sint32,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 6: {
            int64_t tmp;
            status = ast_read_primitive(rt,ast_sint64,6,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_sint64,&primrepeated_v->f_sint64,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 7: {
            uint32_t tmp;
            status = ast_read_primitive(rt,ast_fixed32,7,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_fixed32,&primrepeated_v->f_fixed32,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 8: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_fixed64,8,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_fixed64,&primrepeated_v->f_fixed64,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 9: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_sfixed32,9,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_sfixed32,&primrepeated_v->f_sfixed32,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 10: {
            int64_t tmp;
            status = ast_read_primitive(rt,ast_sfixed64,10,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_sfixed64,&primrepeated_v->f_sfixed64,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 11: {
            double tmp;
            status = ast_read_primitive(rt,ast_double,11,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_double,&primrepeated_v->f_double,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 12: {
            float tmp;
            status = ast_read_primitive(rt,ast_float,12,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_float,&primrepeated_v->f_float,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {goto done;}
    if(primrepeated_vp) *primrepeated_vp = primrepeated_v;
done:
    return status;
} /*PrimRepeated_read*/

ast_err
PrimRepeated_reclaim(ast_runtime* rt, PrimRepeated* primrepeated_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)primrepeated_v);
    goto done;

done:
    return status;

} /*PrimRepeated_reclaim*/

size_t
PrimRepeated_get_size(ast_runtime* rt, PrimRepeated* primrepeated_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<primrepeated_v->f_int32.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_int32,&primrepeated_v->f_int32.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_int64.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
            fieldsize += ast_get_size(rt,ast_int64,&primrepeated_v->f_int64.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_uint32.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint32,&primrepeated_v->f_uint32.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_uint64.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&primrepeated_v->f_uint64.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_sint32.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_sint32,&primrepeated_v->f_sint32.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_sint64.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
            fieldsize += ast_get_size(rt,ast_sint64,&primrepeated_v->f_sint64.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_fixed32.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,7);
            fieldsize += ast_get_size(rt,ast_fixed32,&primrepeated_v->f_fixed32.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_fixed64.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,8);
            fieldsize += ast_get_size(rt,ast_fixed64,&primrepeated_v->f_fixed64.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_sfixed32.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,9);
            fieldsize += ast_get_size(rt,ast_sfixed32,&primrepeated_v->f_sfixed32.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_sfixed64.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,10);
            fieldsize += ast_get_size(rt,ast_sfixed64,&primrepeated_v->f_sfixed64.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_double.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,11);
            fieldsize += ast_get_size(rt,ast_double,&primrepeated_v->f_double.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<primrepeated_v->f_float.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,12);
            fieldsize += ast_get_size(rt,ast_float,&primrepeated_v->f_float.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*PrimRepeated_get_size*/

ast_err
PrimRequired_write(ast_runtime* rt, PrimRequired* primrequired_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&primrequired_v->f_int32);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int64,2,&primrequired_v->f_int64);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint32,3,&primrequired_v->f_uint32);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,4,&primrequired_v->f_uint64);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_sint32,5,&primrequired_v->f_sint32);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_sint64,6,&primrequired_v->f_sint64);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_fixed32,7,&primrequired_v->f_fixed32);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_fixed64,8,&primrequired_v->f_fixed64);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_sfixed32,9,&primrequired_v->f_sfixed32);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_sfixed64,10,&primrequired_v->f_sfixed64);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_double,11,&primrequired_v->f_double);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_float,12,&primrequired_v->f_float);
        if(status != AST_NOERR) {goto done;}
    }

done:
    return status;

} /*PrimRequired_write*/

ast_err
PrimRequired_read(ast_runtime* rt, PrimRequired** primrequired_vp)
{
    ast_err status = AST_NOERR;
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
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {goto done;}
    if(primrequired_vp) *primrequired_vp = primrequired_v;
done:
    return status;
} /*PrimRequired_read*/

ast_err
PrimRequired_reclaim(ast_runtime* rt, PrimRequired* primrequired_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)primrequired_v);
    goto done;

done:
    return status;

} /*PrimRequired_reclaim*/

size_t
PrimRequired_get_size(ast_runtime* rt, PrimRequired* primrequired_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&primrequired_v->f_int32);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int64,&primrequired_v->f_int64);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint32,&primrequired_v->f_uint32);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_uint64,&primrequired_v->f_uint64);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_sint32,&primrequired_v->f_sint32);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_sint64,&primrequired_v->f_sint64);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,7);
        fieldsize += ast_get_size(rt,ast_fixed32,&primrequired_v->f_fixed32);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,8);
        fieldsize += ast_get_size(rt,ast_fixed64,&primrequired_v->f_fixed64);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,9);
        fieldsize += ast_get_size(rt,ast_sfixed32,&primrequired_v->f_sfixed32);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,10);
        fieldsize += ast_get_size(rt,ast_sfixed64,&primrequired_v->f_sfixed64);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,11);
        fieldsize += ast_get_size(rt,ast_double,&primrequired_v->f_double);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,12);
        fieldsize += ast_get_size(rt,ast_float,&primrequired_v->f_float);
        totalsize += fieldsize;
    }
    return totalsize;

} /*PrimRequired_get_size*/

ast_err
PrimOptional_write(ast_runtime* rt, PrimOptional* primoptional_v)
{
    ast_err status = AST_NOERR;

    {
        if(primoptional_v->f_int32.defined) {
            status = ast_write_primitive(rt,ast_int32,1,&primoptional_v->f_int32.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_int64.defined) {
            status = ast_write_primitive(rt,ast_int64,2,&primoptional_v->f_int64.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_uint32.defined) {
            status = ast_write_primitive(rt,ast_uint32,3,&primoptional_v->f_uint32.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_uint64.defined) {
            status = ast_write_primitive(rt,ast_uint64,4,&primoptional_v->f_uint64.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_sint32.defined) {
            status = ast_write_primitive(rt,ast_sint32,5,&primoptional_v->f_sint32.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_sint64.defined) {
            status = ast_write_primitive(rt,ast_sint64,6,&primoptional_v->f_sint64.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_fixed32.defined) {
            status = ast_write_primitive(rt,ast_fixed32,7,&primoptional_v->f_fixed32.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_fixed64.defined) {
            status = ast_write_primitive(rt,ast_fixed64,8,&primoptional_v->f_fixed64.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_sfixed32.defined) {
            status = ast_write_primitive(rt,ast_sfixed32,9,&primoptional_v->f_sfixed32.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_sfixed64.defined) {
            status = ast_write_primitive(rt,ast_sfixed64,10,&primoptional_v->f_sfixed64.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_double.defined) {
            status = ast_write_primitive(rt,ast_double,11,&primoptional_v->f_double.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(primoptional_v->f_float.defined) {
            status = ast_write_primitive(rt,ast_float,12,&primoptional_v->f_float.value);
            if(status != AST_NOERR) {goto done;}
        }
    }

done:
    return status;

} /*PrimOptional_write*/

ast_err
PrimOptional_read(ast_runtime* rt, PrimOptional** primoptional_vp)
{
    ast_err status = AST_NOERR;
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
            primoptional_v->f_int32.defined = 1;
            primoptional_v->f_int32.value = 0;
            status = ast_read_primitive(rt,ast_int32,1,&primoptional_v->f_int32.value);
            } break;
        case 2: {
            primoptional_v->f_int64.defined = 1;
            primoptional_v->f_int64.value = 0;
            status = ast_read_primitive(rt,ast_int64,2,&primoptional_v->f_int64.value);
            } break;
        case 3: {
            primoptional_v->f_uint32.defined = 1;
            primoptional_v->f_uint32.value = 0;
            status = ast_read_primitive(rt,ast_uint32,3,&primoptional_v->f_uint32.value);
            } break;
        case 4: {
            primoptional_v->f_uint64.defined = 1;
            primoptional_v->f_uint64.value = 0;
            status = ast_read_primitive(rt,ast_uint64,4,&primoptional_v->f_uint64.value);
            } break;
        case 5: {
            primoptional_v->f_sint32.defined = 1;
            primoptional_v->f_sint32.value = 0;
            status = ast_read_primitive(rt,ast_sint32,5,&primoptional_v->f_sint32.value);
            } break;
        case 6: {
            primoptional_v->f_sint64.defined = 1;
            primoptional_v->f_sint64.value = 0;
            status = ast_read_primitive(rt,ast_sint64,6,&primoptional_v->f_sint64.value);
            } break;
        case 7: {
            primoptional_v->f_fixed32.defined = 1;
            primoptional_v->f_fixed32.value = 0;
            status = ast_read_primitive(rt,ast_fixed32,7,&primoptional_v->f_fixed32.value);
            } break;
        case 8: {
            primoptional_v->f_fixed64.defined = 1;
            primoptional_v->f_fixed64.value = 0;
            status = ast_read_primitive(rt,ast_fixed64,8,&primoptional_v->f_fixed64.value);
            } break;
        case 9: {
            primoptional_v->f_sfixed32.defined = 1;
            primoptional_v->f_sfixed32.value = 0;
            status = ast_read_primitive(rt,ast_sfixed32,9,&primoptional_v->f_sfixed32.value);
            } break;
        case 10: {
            primoptional_v->f_sfixed64.defined = 1;
            primoptional_v->f_sfixed64.value = 0;
            status = ast_read_primitive(rt,ast_sfixed64,10,&primoptional_v->f_sfixed64.value);
            } break;
        case 11: {
            primoptional_v->f_double.defined = 1;
            primoptional_v->f_double.value = 0;
            status = ast_read_primitive(rt,ast_double,11,&primoptional_v->f_double.value);
            } break;
        case 12: {
            primoptional_v->f_float.defined = 1;
            primoptional_v->f_float.value = 0;
            status = ast_read_primitive(rt,ast_float,12,&primoptional_v->f_float.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(!primoptional_v->f_int32.defined) {
        primoptional_v->f_int32.value = 0;
    }
    if(!primoptional_v->f_int64.defined) {
        primoptional_v->f_int64.value = 0;
    }
    if(!primoptional_v->f_uint32.defined) {
        primoptional_v->f_uint32.value = 0;
    }
    if(!primoptional_v->f_uint64.defined) {
        primoptional_v->f_uint64.value = 0;
    }
    if(!primoptional_v->f_sint32.defined) {
        primoptional_v->f_sint32.value = 0;
    }
    if(!primoptional_v->f_sint64.defined) {
        primoptional_v->f_sint64.value = 0;
    }
    if(!primoptional_v->f_fixed32.defined) {
        primoptional_v->f_fixed32.value = 0;
    }
    if(!primoptional_v->f_fixed64.defined) {
        primoptional_v->f_fixed64.value = 0;
    }
    if(!primoptional_v->f_sfixed32.defined) {
        primoptional_v->f_sfixed32.value = 0;
    }
    if(!primoptional_v->f_sfixed64.defined) {
        primoptional_v->f_sfixed64.value = 0;
    }
    if(!primoptional_v->f_double.defined) {
        primoptional_v->f_double.value = 0;
    }
    if(!primoptional_v->f_float.defined) {
        primoptional_v->f_float.value = 0;
    }
    if(status != AST_NOERR) {goto done;}
    if(primoptional_vp) *primoptional_vp = primoptional_v;
done:
    return status;
} /*PrimOptional_read*/

ast_err
PrimOptional_reclaim(ast_runtime* rt, PrimOptional* primoptional_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)primoptional_v);
    goto done;

done:
    return status;

} /*PrimOptional_reclaim*/

size_t
PrimOptional_get_size(ast_runtime* rt, PrimOptional* primoptional_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        if(primoptional_v->f_int32.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_int32,&primoptional_v->f_int32.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_int64.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
            fieldsize += ast_get_size(rt,ast_int64,&primoptional_v->f_int64.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_uint32.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint32,&primoptional_v->f_uint32.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_uint64.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&primoptional_v->f_uint64.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_sint32.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_sint32,&primoptional_v->f_sint32.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_sint64.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
            fieldsize += ast_get_size(rt,ast_sint64,&primoptional_v->f_sint64.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_fixed32.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,7);
            fieldsize += ast_get_size(rt,ast_fixed32,&primoptional_v->f_fixed32.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_fixed64.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,8);
            fieldsize += ast_get_size(rt,ast_fixed64,&primoptional_v->f_fixed64.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_sfixed32.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,9);
            fieldsize += ast_get_size(rt,ast_sfixed32,&primoptional_v->f_sfixed32.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_sfixed64.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,10);
            fieldsize += ast_get_size(rt,ast_sfixed64,&primoptional_v->f_sfixed64.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_double.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,11);
            fieldsize += ast_get_size(rt,ast_double,&primoptional_v->f_double.value);
        }
        totalsize += fieldsize;
    }
    {
        if(primoptional_v->f_float.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,12);
            fieldsize += ast_get_size(rt,ast_float,&primoptional_v->f_float.value);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*PrimOptional_get_size*/

