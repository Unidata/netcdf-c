#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "Bytetests.h"

ast_err
BytesRequired_write(ast_runtime* rt, BytesRequired* bytesrequired_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&bytesrequired_v->f_string);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,2,&bytesrequired_v->f_bytes);
        if(status != AST_NOERR) {goto done;}
    }

done:
    return status;

} /*BytesRequired_write*/

ast_err
BytesRequired_read(ast_runtime* rt, BytesRequired** bytesrequired_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    BytesRequired* bytesrequired_v;

    bytesrequired_v = (BytesRequired*)ast_alloc(rt,sizeof(BytesRequired));
    if(bytesrequired_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&bytesrequired_v->f_string);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bytes,2,&bytesrequired_v->f_bytes);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {goto done;}
    if(bytesrequired_vp) *bytesrequired_vp = bytesrequired_v;
done:
    return status;
} /*BytesRequired_read*/

ast_err
BytesRequired_reclaim(ast_runtime* rt, BytesRequired* bytesrequired_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,bytesrequired_v->f_string);
        if(status != AST_NOERR) {goto done;}
    }
    {
        status = ast_reclaim_bytes(rt,&bytesrequired_v->f_bytes);
        if(status != AST_NOERR) {goto done;}
    }
    ast_free(rt,(void*)bytesrequired_v);
    goto done;

done:
    return status;

} /*BytesRequired_reclaim*/

size_t
BytesRequired_get_size(ast_runtime* rt, BytesRequired* bytesrequired_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&bytesrequired_v->f_string);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bytes,&bytesrequired_v->f_bytes);
        totalsize += fieldsize;
    }
    return totalsize;

} /*BytesRequired_get_size*/

ast_err
BytesOptional_write(ast_runtime* rt, BytesOptional* bytesoptional_v)
{
    ast_err status = AST_NOERR;

    {
        if(bytesoptional_v->f_string.defined) {
            status = ast_write_primitive(rt,ast_string,1,&bytesoptional_v->f_string.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(bytesoptional_v->f_bytes.defined) {
            status = ast_write_primitive(rt,ast_bytes,2,&bytesoptional_v->f_bytes.value);
            if(status != AST_NOERR) {goto done;}
        }
    }

done:
    return status;

} /*BytesOptional_write*/

ast_err
BytesOptional_read(ast_runtime* rt, BytesOptional** bytesoptional_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    BytesOptional* bytesoptional_v;

    bytesoptional_v = (BytesOptional*)ast_alloc(rt,sizeof(BytesOptional));
    if(bytesoptional_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            bytesoptional_v->f_string.defined = 1;
            bytesoptional_v->f_string.value = NULL;
            status = ast_read_primitive(rt,ast_string,1,&bytesoptional_v->f_string.value);
            } break;
        case 2: {
            bytesoptional_v->f_bytes.defined = 1;
            bytesoptional_v->f_bytes.value.nbytes = 0;
            bytesoptional_v->f_bytes.value.bytes = NULL;
            status = ast_read_primitive(rt,ast_bytes,2,&bytesoptional_v->f_bytes.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(!bytesoptional_v->f_string.defined) {
        bytesoptional_v->f_string.value = "hello";
    }
    if(!bytesoptional_v->f_bytes.defined) {
        bytesoptional_v->f_bytes.value.nbytes = 0;
        bytesoptional_v->f_bytes.value.bytes = NULL;
    }
    if(status != AST_NOERR) {goto done;}
    if(bytesoptional_vp) *bytesoptional_vp = bytesoptional_v;
done:
    return status;
} /*BytesOptional_read*/

ast_err
BytesOptional_reclaim(ast_runtime* rt, BytesOptional* bytesoptional_v)
{
    ast_err status = AST_NOERR;

    {
        if(bytesoptional_v->f_string.defined) {
            status = ast_reclaim_string(rt,bytesoptional_v->f_string.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        if(bytesoptional_v->f_bytes.defined) {
            status = ast_reclaim_bytes(rt,&bytesoptional_v->f_bytes.value);
            if(status != AST_NOERR) {goto done;}
        }
    }
    ast_free(rt,(void*)bytesoptional_v);
    goto done;

done:
    return status;

} /*BytesOptional_reclaim*/

size_t
BytesOptional_get_size(ast_runtime* rt, BytesOptional* bytesoptional_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        if(bytesoptional_v->f_string.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_string,&bytesoptional_v->f_string.value);
        }
        totalsize += fieldsize;
    }
    {
        if(bytesoptional_v->f_bytes.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
            fieldsize += ast_get_size(rt,ast_bytes,&bytesoptional_v->f_bytes.value);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*BytesOptional_get_size*/

ast_err
BytesRepeated_write(ast_runtime* rt, BytesRepeated* bytesrepeated_v)
{
    ast_err status = AST_NOERR;

    {
        int i = 0;
        for(i=0;i<bytesrepeated_v->f_string.count;i++) {
            status = ast_write_primitive(rt,ast_string,1,&bytesrepeated_v->f_string.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<bytesrepeated_v->f_bytes.count;i++) {
            status = ast_write_primitive(rt,ast_bytes,2,&bytesrepeated_v->f_bytes.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
    }

done:
    return status;

} /*BytesRepeated_write*/

ast_err
BytesRepeated_read(ast_runtime* rt, BytesRepeated** bytesrepeated_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    BytesRepeated* bytesrepeated_v;

    bytesrepeated_v = (BytesRepeated*)ast_alloc(rt,sizeof(BytesRepeated));
    if(bytesrepeated_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        switch (fieldno) {
        case 1: {
            char* tmp;
            status = ast_read_primitive(rt,ast_string,1,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_string,&bytesrepeated_v->f_string,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        case 2: {
            bytes_t tmp;
            status = ast_read_primitive(rt,ast_bytes,2,&tmp);
            if(status != AST_NOERR) {goto done;}
            status = ast_repeat_append(rt,ast_bytes,&bytesrepeated_v->f_bytes,&tmp);
            if(status != AST_NOERR) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {goto done;}
    if(bytesrepeated_vp) *bytesrepeated_vp = bytesrepeated_v;
done:
    return status;
} /*BytesRepeated_read*/

ast_err
BytesRepeated_reclaim(ast_runtime* rt, BytesRepeated* bytesrepeated_v)
{
    ast_err status = AST_NOERR;

    {
        int i;
        for(i=0;i<bytesrepeated_v->f_string.count;i++) {
            status = ast_reclaim_string(rt,bytesrepeated_v->f_string.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
        ast_free(rt,bytesrepeated_v->f_string.values);
    }
    {
        int i;
        for(i=0;i<bytesrepeated_v->f_bytes.count;i++) {
            status = ast_reclaim_bytes(rt,&bytesrepeated_v->f_bytes.values[i]);
            if(status != AST_NOERR) {goto done;}
        }
        ast_free(rt,bytesrepeated_v->f_bytes.values);
    }
    ast_free(rt,(void*)bytesrepeated_v);
    goto done;

done:
    return status;

} /*BytesRepeated_reclaim*/

size_t
BytesRepeated_get_size(ast_runtime* rt, BytesRepeated* bytesrepeated_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<bytesrepeated_v->f_string.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_string,&bytesrepeated_v->f_string.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<bytesrepeated_v->f_bytes.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
            fieldsize += ast_get_size(rt,ast_bytes,&bytesrepeated_v->f_bytes.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*BytesRepeated_get_size*/

