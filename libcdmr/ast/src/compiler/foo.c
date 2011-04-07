
#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>

#include "foo.h"

int
SubMess_write(ast_runtime* rt, SubMess* submess_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_int32,4,&submess_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*SubMess_write*/

int
SubMess_read(ast_runtime* rt, SubMess** submess_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    SubMess* submess_v;

    submess_v = (SubMess*)ast_alloc(sizeof(SubMess));
    if(submess_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 4: {
            status = ast_read(rt,ast_int32,&submess_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(submess_v->test_f.exists) {
    submess_v->test_f.value = 0;
}
done:
    return status;
} /*SubMess_read*/

int
SubMess_reclaim(ast_runtime* rt, SubMess* submess_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*SubMess_reclaim*/

long
SubMess_size(ast_runtime* rt, SubMess* submess_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_int32,&submess_v->test_f);
    return status;

} /*SubMess_write_size*/
int
TestFieldNo15_write(ast_runtime* rt, TestFieldNo15* testfieldno15_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,15,&testfieldno15_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo15_write*/

int
TestFieldNo15_read(ast_runtime* rt, TestFieldNo15** testfieldno15_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestFieldNo15* testfieldno15_v;

    testfieldno15_v = (TestFieldNo15*)ast_alloc(sizeof(TestFieldNo15));
    if(testfieldno15_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 15: {
            status = ast_read(rt,ast_string,&testfieldno15_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestFieldNo15_read*/

int
TestFieldNo15_reclaim(ast_runtime* rt, TestFieldNo15* testfieldno15_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testfieldno15_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo15_reclaim*/

long
TestFieldNo15_size(ast_runtime* rt, TestFieldNo15* testfieldno15_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testfieldno15_v->test_f);
    return status;

} /*TestFieldNo15_write_size*/
int
TestFieldNo16_write(ast_runtime* rt, TestFieldNo16* testfieldno16_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,16,&testfieldno16_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo16_write*/

int
TestFieldNo16_read(ast_runtime* rt, TestFieldNo16** testfieldno16_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestFieldNo16* testfieldno16_v;

    testfieldno16_v = (TestFieldNo16*)ast_alloc(sizeof(TestFieldNo16));
    if(testfieldno16_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 16: {
            status = ast_read(rt,ast_string,&testfieldno16_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestFieldNo16_read*/

int
TestFieldNo16_reclaim(ast_runtime* rt, TestFieldNo16* testfieldno16_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testfieldno16_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo16_reclaim*/

long
TestFieldNo16_size(ast_runtime* rt, TestFieldNo16* testfieldno16_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testfieldno16_v->test_f);
    return status;

} /*TestFieldNo16_write_size*/
int
TestFieldNo2047_write(ast_runtime* rt, TestFieldNo2047* testfieldno2047_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,2047,&testfieldno2047_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo2047_write*/

int
TestFieldNo2047_read(ast_runtime* rt, TestFieldNo2047** testfieldno2047_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestFieldNo2047* testfieldno2047_v;

    testfieldno2047_v = (TestFieldNo2047*)ast_alloc(sizeof(TestFieldNo2047));
    if(testfieldno2047_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 2047: {
            status = ast_read(rt,ast_string,&testfieldno2047_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestFieldNo2047_read*/

int
TestFieldNo2047_reclaim(ast_runtime* rt, TestFieldNo2047* testfieldno2047_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testfieldno2047_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo2047_reclaim*/

long
TestFieldNo2047_size(ast_runtime* rt, TestFieldNo2047* testfieldno2047_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testfieldno2047_v->test_f);
    return status;

} /*TestFieldNo2047_write_size*/
int
TestFieldNo2048_write(ast_runtime* rt, TestFieldNo2048* testfieldno2048_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,2048,&testfieldno2048_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo2048_write*/

int
TestFieldNo2048_read(ast_runtime* rt, TestFieldNo2048** testfieldno2048_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestFieldNo2048* testfieldno2048_v;

    testfieldno2048_v = (TestFieldNo2048*)ast_alloc(sizeof(TestFieldNo2048));
    if(testfieldno2048_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 2048: {
            status = ast_read(rt,ast_string,&testfieldno2048_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestFieldNo2048_read*/

int
TestFieldNo2048_reclaim(ast_runtime* rt, TestFieldNo2048* testfieldno2048_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testfieldno2048_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo2048_reclaim*/

long
TestFieldNo2048_size(ast_runtime* rt, TestFieldNo2048* testfieldno2048_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testfieldno2048_v->test_f);
    return status;

} /*TestFieldNo2048_write_size*/
int
TestFieldNo262143_write(ast_runtime* rt, TestFieldNo262143* testfieldno262143_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,262143,&testfieldno262143_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo262143_write*/

int
TestFieldNo262143_read(ast_runtime* rt, TestFieldNo262143** testfieldno262143_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestFieldNo262143* testfieldno262143_v;

    testfieldno262143_v = (TestFieldNo262143*)ast_alloc(sizeof(TestFieldNo262143));
    if(testfieldno262143_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 262143: {
            status = ast_read(rt,ast_string,&testfieldno262143_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestFieldNo262143_read*/

int
TestFieldNo262143_reclaim(ast_runtime* rt, TestFieldNo262143* testfieldno262143_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testfieldno262143_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo262143_reclaim*/

long
TestFieldNo262143_size(ast_runtime* rt, TestFieldNo262143* testfieldno262143_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testfieldno262143_v->test_f);
    return status;

} /*TestFieldNo262143_write_size*/
int
TestFieldNo262144_write(ast_runtime* rt, TestFieldNo262144* testfieldno262144_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,262144,&testfieldno262144_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo262144_write*/

int
TestFieldNo262144_read(ast_runtime* rt, TestFieldNo262144** testfieldno262144_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestFieldNo262144* testfieldno262144_v;

    testfieldno262144_v = (TestFieldNo262144*)ast_alloc(sizeof(TestFieldNo262144));
    if(testfieldno262144_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 262144: {
            status = ast_read(rt,ast_string,&testfieldno262144_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestFieldNo262144_read*/

int
TestFieldNo262144_reclaim(ast_runtime* rt, TestFieldNo262144* testfieldno262144_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testfieldno262144_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo262144_reclaim*/

long
TestFieldNo262144_size(ast_runtime* rt, TestFieldNo262144* testfieldno262144_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testfieldno262144_v->test_f);
    return status;

} /*TestFieldNo262144_write_size*/
int
TestFieldNo33554431_write(ast_runtime* rt, TestFieldNo33554431* testfieldno33554431_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,33554431,&testfieldno33554431_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo33554431_write*/

int
TestFieldNo33554431_read(ast_runtime* rt, TestFieldNo33554431** testfieldno33554431_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestFieldNo33554431* testfieldno33554431_v;

    testfieldno33554431_v = (TestFieldNo33554431*)ast_alloc(sizeof(TestFieldNo33554431));
    if(testfieldno33554431_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 33554431: {
            status = ast_read(rt,ast_string,&testfieldno33554431_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestFieldNo33554431_read*/

int
TestFieldNo33554431_reclaim(ast_runtime* rt, TestFieldNo33554431* testfieldno33554431_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testfieldno33554431_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo33554431_reclaim*/

long
TestFieldNo33554431_size(ast_runtime* rt, TestFieldNo33554431* testfieldno33554431_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testfieldno33554431_v->test_f);
    return status;

} /*TestFieldNo33554431_write_size*/
int
TestFieldNo33554432_write(ast_runtime* rt, TestFieldNo33554432* testfieldno33554432_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,33554432,&testfieldno33554432_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo33554432_write*/

int
TestFieldNo33554432_read(ast_runtime* rt, TestFieldNo33554432** testfieldno33554432_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestFieldNo33554432* testfieldno33554432_v;

    testfieldno33554432_v = (TestFieldNo33554432*)ast_alloc(sizeof(TestFieldNo33554432));
    if(testfieldno33554432_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 33554432: {
            status = ast_read(rt,ast_string,&testfieldno33554432_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestFieldNo33554432_read*/

int
TestFieldNo33554432_reclaim(ast_runtime* rt, TestFieldNo33554432* testfieldno33554432_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testfieldno33554432_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestFieldNo33554432_reclaim*/

long
TestFieldNo33554432_size(ast_runtime* rt, TestFieldNo33554432* testfieldno33554432_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testfieldno33554432_v->test_f);
    return status;

} /*TestFieldNo33554432_write_size*/
int
TestMess_write(ast_runtime* rt, TestMess* testmess_v)
{
    int status = AST_NOERR;
    int i = 0;

    for(i=0;i<testmess_v->test_int32_f.count;i++) {
        status = ast_write(rt,ast_int32,1,&testmess_v->test_int32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_sint32_f.count;i++) {
        status = ast_write(rt,ast_sint32,2,&testmess_v->test_sint32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_sfixed32_f.count;i++) {
        status = ast_write(rt,ast_sfixed32,3,&testmess_v->test_sfixed32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_int64_f.count;i++) {
        status = ast_write(rt,ast_int64,4,&testmess_v->test_int64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_sint64_f.count;i++) {
        status = ast_write(rt,ast_sint64,5,&testmess_v->test_sint64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_sfixed64_f.count;i++) {
        status = ast_write(rt,ast_sfixed64,6,&testmess_v->test_sfixed64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_uint32_f.count;i++) {
        status = ast_write(rt,ast_uint32,7,&testmess_v->test_uint32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_fixed32_f.count;i++) {
        status = ast_write(rt,ast_fixed32,8,&testmess_v->test_fixed32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_uint64_f.count;i++) {
        status = ast_write(rt,ast_uint64,9,&testmess_v->test_uint64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_fixed64_f.count;i++) {
        status = ast_write(rt,ast_fixed64,10,&testmess_v->test_fixed64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_float_f.count;i++) {
        status = ast_write(rt,ast_float,11,&testmess_v->test_float_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_double_f.count;i++) {
        status = ast_write(rt,ast_double,12,&testmess_v->test_double_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_boolean_f.count;i++) {
        status = ast_write(rt,ast_bool,13,&testmess_v->test_boolean_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_enum_small_f.count;i++) {
        status = TestEnumSmall_write(rt,testmess_v->test_enum_small_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_enum_f.count;i++) {
        status = TestEnum_write(rt,testmess_v->test_enum_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_string_f.count;i++) {
        status = ast_write(rt,ast_string,16,&testmess_v->test_string_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_bytes_f.count;i++) {
        status = ast_write(rt,ast_bytes,17,&testmess_v->test_bytes_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmess_v->test_message_f.count;i++) {
        status = SubMess_write(rt,testmess_v->test_message_f.values[i]);
        if(!status) {goto done;}
    }

done:
    return status;

} /*TestMess_write*/

int
TestMess_read(ast_runtime* rt, TestMess** testmess_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestMess* testmess_v;

    testmess_v = (TestMess*)ast_alloc(sizeof(TestMess));
    if(testmess_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            testmess_v->test_int32_f.count = 0;
            testmess_v->test_int32_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_int32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_int32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_int32,&tmp);
                status = ast_append(rt,ast_int32,&testmess_v->test_int32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 2: {
            testmess_v->test_sint32_f.count = 0;
            testmess_v->test_sint32_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_sint32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_sint32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_sint32,&tmp);
                status = ast_append(rt,ast_sint32,&testmess_v->test_sint32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 3: {
            testmess_v->test_sfixed32_f.count = 0;
            testmess_v->test_sfixed32_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_sfixed32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_sfixed32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_sfixed32,&tmp);
                status = ast_append(rt,ast_sfixed32,&testmess_v->test_sfixed32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 4: {
            testmess_v->test_int64_f.count = 0;
            testmess_v->test_int64_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_int64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_int64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_int64,&tmp);
                status = ast_append(rt,ast_int64,&testmess_v->test_int64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 5: {
            testmess_v->test_sint64_f.count = 0;
            testmess_v->test_sint64_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_sint64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_sint64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_sint64,&tmp);
                status = ast_append(rt,ast_sint64,&testmess_v->test_sint64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 6: {
            testmess_v->test_sfixed64_f.count = 0;
            testmess_v->test_sfixed64_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_sfixed64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_sfixed64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_sfixed64,&tmp);
                status = ast_append(rt,ast_sfixed64,&testmess_v->test_sfixed64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 7: {
            testmess_v->test_uint32_f.count = 0;
            testmess_v->test_uint32_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_uint32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_uint32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_uint32,&tmp);
                status = ast_append(rt,ast_uint32,&testmess_v->test_uint32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 8: {
            testmess_v->test_fixed32_f.count = 0;
            testmess_v->test_fixed32_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_fixed32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_fixed32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_fixed32,&tmp);
                status = ast_append(rt,ast_fixed32,&testmess_v->test_fixed32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 9: {
            testmess_v->test_uint64_f.count = 0;
            testmess_v->test_uint64_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_uint64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_uint64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_uint64,&tmp);
                status = ast_append(rt,ast_uint64,&testmess_v->test_uint64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 10: {
            testmess_v->test_fixed64_f.count = 0;
            testmess_v->test_fixed64_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_fixed64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_fixed64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_fixed64,&tmp);
                status = ast_append(rt,ast_fixed64,&testmess_v->test_fixed64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 11: {
            testmess_v->test_float_f.count = 0;
            testmess_v->test_float_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_float_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_float_f.count;i++) {
                float tmp;
                status = ast_read(rt,ast_float,&tmp);
                status = ast_append(rt,ast_float,&testmess_v->test_float_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 12: {
            testmess_v->test_double_f.count = 0;
            testmess_v->test_double_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_double_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_double_f.count;i++) {
                double tmp;
                status = ast_read(rt,ast_double,&tmp);
                status = ast_append(rt,ast_double,&testmess_v->test_double_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 13: {
            testmess_v->test_boolean_f.count = 0;
            testmess_v->test_boolean_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_boolean_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_boolean_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_bool,&tmp);
                status = ast_append(rt,ast_bool,&testmess_v->test_boolean_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 14: {
            testmess_v->test_enum_small_f.count = 0;
            testmess_v->test_enum_small_f.values = NULL;
            status = ast_read(rt,ast_int32,testmess_v->test_enum_small_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_enum_small_f.count;i++) {
                TestEnumSmall* tmp;
                status = TestEnumSmall_read(rt,&tmp);
                status = ast_append(rt,ast_enum,&testmess_v->test_enum_small_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 15: {
            testmess_v->test_enum_f.count = 0;
            testmess_v->test_enum_f.values = NULL;
            status = ast_read(rt,ast_int32,testmess_v->test_enum_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_enum_f.count;i++) {
                TestEnum* tmp;
                status = TestEnum_read(rt,&tmp);
                status = ast_append(rt,ast_enum,&testmess_v->test_enum_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 16: {
            testmess_v->test_string_f.count = 0;
            testmess_v->test_string_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_string_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_string_f.count;i++) {
                char* tmp;
                status = ast_read(rt,ast_string,&tmp);
                status = ast_append(rt,ast_string,&testmess_v->test_string_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 17: {
            testmess_v->test_bytes_f.count = 0;
            testmess_v->test_bytes_f.values = NULL;
            status = ast_read_count(rt,&testmess_v->test_bytes_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_bytes_f.count;i++) {
                Bytestring tmp;
                status = ast_read(rt,ast_bytes,&tmp);
                status = ast_append(rt,ast_bytes,&testmess_v->test_bytes_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 18: {
            testmess_v->test_message_f.count = 0;
            testmess_v->test_message_f.values = NULL;
            status = ast_read(rt,ast_int32,testmess_v->test_message_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmess_v->test_message_f.count;i++) {
                SubMess* tmp;
                status = SubMess_read(rt,&tmp);
                status = ast_append(rt,ast_message,&testmess_v->test_message_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmess_v->test_int32_f.exists) {
    testmess_v->test_int32_f.value = 0;
}
if(testmess_v->test_sint32_f.exists) {
    testmess_v->test_sint32_f.value = 0;
}
if(testmess_v->test_sfixed32_f.exists) {
    testmess_v->test_sfixed32_f.value = 0;
}
if(testmess_v->test_int64_f.exists) {
    testmess_v->test_int64_f.value = 0;
}
if(testmess_v->test_sint64_f.exists) {
    testmess_v->test_sint64_f.value = 0;
}
if(testmess_v->test_sfixed64_f.exists) {
    testmess_v->test_sfixed64_f.value = 0;
}
if(testmess_v->test_uint32_f.exists) {
    testmess_v->test_uint32_f.value = 0;
}
if(testmess_v->test_fixed32_f.exists) {
    testmess_v->test_fixed32_f.value = 0;
}
if(testmess_v->test_uint64_f.exists) {
    testmess_v->test_uint64_f.value = 0;
}
if(testmess_v->test_fixed64_f.exists) {
    testmess_v->test_fixed64_f.value = 0;
}
if(testmess_v->test_float_f.exists) {
    testmess_v->test_float_f.value = 0;
}
if(testmess_v->test_double_f.exists) {
    testmess_v->test_double_f.value = 0;
}
if(testmess_v->test_boolean_f.exists) {
    testmess_v->test_boolean_f.value = 0;
}
done:
    return status;
} /*TestMess_read*/

int
TestMess_reclaim(ast_runtime* rt, TestMess* testmess_v)
{
    int status = AST_NOERR;
    int i = 0;

    for(i=0;i<testmess_v->test_string_f.count;i++) {
        status = ast_reclaim(rt,ast_string,&testmess_v->test_string_f.values[i]);
        if(!status) {goto done;}
    }
    ast_free(TestMess->test_string.values)
    for(i=0;i<testmess_v->test_bytes_f.count;i++) {
        status = ast_reclaim(rt,ast_bytes,&testmess_v->test_bytes_f.values[i]);
        if(!status) {goto done;}
    }
    ast_free(TestMess->test_bytes.values)
    for(i=0;i<testmess_v->test_message_f.count;i++) {
        status = SubMess_reclaim(rt,testmess_v->test_message_f.values[i]);
        if(!status) {goto done;}
    }
    ast_free(TestMess->test_message.values)

done:
    return status;

} /*TestMess_reclaim*/

long
TestMess_size(ast_runtime* rt, TestMess* testmess_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    for(i=0;i<testmess_v->test_int32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_int32,testmess_v->test_int32_f.values[i]);
    }
    for(i=0;i<testmess_v->test_sint32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sint32,testmess_v->test_sint32_f.values[i]);
    }
    for(i=0;i<testmess_v->test_sfixed32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sfixed32,testmess_v->test_sfixed32_f.values[i]);
    }
    for(i=0;i<testmess_v->test_int64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_int64,testmess_v->test_int64_f.values[i]);
    }
    for(i=0;i<testmess_v->test_sint64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sint64,testmess_v->test_sint64_f.values[i]);
    }
    for(i=0;i<testmess_v->test_sfixed64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sfixed64,testmess_v->test_sfixed64_f.values[i]);
    }
    for(i=0;i<testmess_v->test_uint32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_uint32,testmess_v->test_uint32_f.values[i]);
    }
    for(i=0;i<testmess_v->test_fixed32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_fixed32,testmess_v->test_fixed32_f.values[i]);
    }
    for(i=0;i<testmess_v->test_uint64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_uint64,testmess_v->test_uint64_f.values[i]);
    }
    for(i=0;i<testmess_v->test_fixed64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_fixed64,testmess_v->test_fixed64_f.values[i]);
    }
    for(i=0;i<testmess_v->test_float_f.count;i++) {
        totalsize += ast_write_size(rt,ast_float,testmess_v->test_float_f.values[i]);
    }
    for(i=0;i<testmess_v->test_double_f.count;i++) {
        totalsize += ast_write_size(rt,ast_double,testmess_v->test_double_f.values[i]);
    }
    for(i=0;i<testmess_v->test_boolean_f.count;i++) {
        totalsize += ast_write_size(rt,ast_bool,testmess_v->test_boolean_f.values[i]);
    }
    for(i=0;i<testmess_v->test_enum_small_f.count;i++) {
        totalsize += TestEnumSmall_write_size(rt,14,&testmess_v->test_enum_small_f);
    }
    for(i=0;i<testmess_v->test_enum_f.count;i++) {
        totalsize += TestEnum_write_size(rt,15,&testmess_v->test_enum_f);
    }
    for(i=0;i<testmess_v->test_string_f.count;i++) {
        totalsize += ast_write_size(rt,ast_string,testmess_v->test_string_f.values[i]);
    }
    for(i=0;i<testmess_v->test_bytes_f.count;i++) {
        totalsize += ast_write_size(rt,ast_bytes,testmess_v->test_bytes_f.values[i]);
    }
    for(i=0;i<testmess_v->test_message_f.count;i++) {
        totalsize += SubMess_write_size(rt,18,&testmess_v->test_message_f);
    }
    return status;

} /*TestMess_write_size*/
int
TestMessPacked_write(ast_runtime* rt, TestMessPacked* testmesspacked_v)
{
    int status = AST_NOERR;
    int i = 0;

    for(i=0;i<testmesspacked_v->test_int32_f.count;i++) {
        status = ast_write(rt,ast_int32,1,&testmesspacked_v->test_int32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_sint32_f.count;i++) {
        status = ast_write(rt,ast_sint32,2,&testmesspacked_v->test_sint32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_sfixed32_f.count;i++) {
        status = ast_write(rt,ast_sfixed32,3,&testmesspacked_v->test_sfixed32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_int64_f.count;i++) {
        status = ast_write(rt,ast_int64,4,&testmesspacked_v->test_int64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_sint64_f.count;i++) {
        status = ast_write(rt,ast_sint64,5,&testmesspacked_v->test_sint64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_sfixed64_f.count;i++) {
        status = ast_write(rt,ast_sfixed64,6,&testmesspacked_v->test_sfixed64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_uint32_f.count;i++) {
        status = ast_write(rt,ast_uint32,7,&testmesspacked_v->test_uint32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_fixed32_f.count;i++) {
        status = ast_write(rt,ast_fixed32,8,&testmesspacked_v->test_fixed32_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_uint64_f.count;i++) {
        status = ast_write(rt,ast_uint64,9,&testmesspacked_v->test_uint64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_fixed64_f.count;i++) {
        status = ast_write(rt,ast_fixed64,10,&testmesspacked_v->test_fixed64_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_float_f.count;i++) {
        status = ast_write(rt,ast_float,11,&testmesspacked_v->test_float_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_double_f.count;i++) {
        status = ast_write(rt,ast_double,12,&testmesspacked_v->test_double_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_boolean_f.count;i++) {
        status = ast_write(rt,ast_bool,13,&testmesspacked_v->test_boolean_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_enum_small_f.count;i++) {
        status = TestEnumSmall_write(rt,testmesspacked_v->test_enum_small_f.values[i]);
        if(!status) {goto done;}
    }
    for(i=0;i<testmesspacked_v->test_enum_f.count;i++) {
        status = TestEnum_write(rt,testmesspacked_v->test_enum_f.values[i]);
        if(!status) {goto done;}
    }

done:
    return status;

} /*TestMessPacked_write*/

int
TestMessPacked_read(ast_runtime* rt, TestMessPacked** testmesspacked_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestMessPacked* testmesspacked_v;

    testmesspacked_v = (TestMessPacked*)ast_alloc(sizeof(TestMessPacked));
    if(testmesspacked_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            testmesspacked_v->test_int32_f.count = 0;
            testmesspacked_v->test_int32_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_int32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_int32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_int32,&tmp);
                status = ast_append(rt,ast_int32,&testmesspacked_v->test_int32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 2: {
            testmesspacked_v->test_sint32_f.count = 0;
            testmesspacked_v->test_sint32_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_sint32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_sint32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_sint32,&tmp);
                status = ast_append(rt,ast_sint32,&testmesspacked_v->test_sint32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 3: {
            testmesspacked_v->test_sfixed32_f.count = 0;
            testmesspacked_v->test_sfixed32_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_sfixed32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_sfixed32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_sfixed32,&tmp);
                status = ast_append(rt,ast_sfixed32,&testmesspacked_v->test_sfixed32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 4: {
            testmesspacked_v->test_int64_f.count = 0;
            testmesspacked_v->test_int64_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_int64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_int64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_int64,&tmp);
                status = ast_append(rt,ast_int64,&testmesspacked_v->test_int64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 5: {
            testmesspacked_v->test_sint64_f.count = 0;
            testmesspacked_v->test_sint64_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_sint64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_sint64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_sint64,&tmp);
                status = ast_append(rt,ast_sint64,&testmesspacked_v->test_sint64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 6: {
            testmesspacked_v->test_sfixed64_f.count = 0;
            testmesspacked_v->test_sfixed64_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_sfixed64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_sfixed64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_sfixed64,&tmp);
                status = ast_append(rt,ast_sfixed64,&testmesspacked_v->test_sfixed64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 7: {
            testmesspacked_v->test_uint32_f.count = 0;
            testmesspacked_v->test_uint32_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_uint32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_uint32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_uint32,&tmp);
                status = ast_append(rt,ast_uint32,&testmesspacked_v->test_uint32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 8: {
            testmesspacked_v->test_fixed32_f.count = 0;
            testmesspacked_v->test_fixed32_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_fixed32_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_fixed32_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_fixed32,&tmp);
                status = ast_append(rt,ast_fixed32,&testmesspacked_v->test_fixed32_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 9: {
            testmesspacked_v->test_uint64_f.count = 0;
            testmesspacked_v->test_uint64_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_uint64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_uint64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_uint64,&tmp);
                status = ast_append(rt,ast_uint64,&testmesspacked_v->test_uint64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 10: {
            testmesspacked_v->test_fixed64_f.count = 0;
            testmesspacked_v->test_fixed64_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_fixed64_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_fixed64_f.count;i++) {
                longlong tmp;
                status = ast_read(rt,ast_fixed64,&tmp);
                status = ast_append(rt,ast_fixed64,&testmesspacked_v->test_fixed64_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 11: {
            testmesspacked_v->test_float_f.count = 0;
            testmesspacked_v->test_float_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_float_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_float_f.count;i++) {
                float tmp;
                status = ast_read(rt,ast_float,&tmp);
                status = ast_append(rt,ast_float,&testmesspacked_v->test_float_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 12: {
            testmesspacked_v->test_double_f.count = 0;
            testmesspacked_v->test_double_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_double_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_double_f.count;i++) {
                double tmp;
                status = ast_read(rt,ast_double,&tmp);
                status = ast_append(rt,ast_double,&testmesspacked_v->test_double_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 13: {
            testmesspacked_v->test_boolean_f.count = 0;
            testmesspacked_v->test_boolean_f.values = NULL;
            status = ast_read_count(rt,&testmesspacked_v->test_boolean_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_boolean_f.count;i++) {
                int tmp;
                status = ast_read(rt,ast_bool,&tmp);
                status = ast_append(rt,ast_bool,&testmesspacked_v->test_boolean_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 14: {
            testmesspacked_v->test_enum_small_f.count = 0;
            testmesspacked_v->test_enum_small_f.values = NULL;
            status = ast_read(rt,ast_int32,testmesspacked_v->test_enum_small_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_enum_small_f.count;i++) {
                TestEnumSmall* tmp;
                status = TestEnumSmall_read(rt,&tmp);
                status = ast_append(rt,ast_enum,&testmesspacked_v->test_enum_small_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 15: {
            testmesspacked_v->test_enum_f.count = 0;
            testmesspacked_v->test_enum_f.values = NULL;
            status = ast_read(rt,ast_int32,testmesspacked_v->test_enum_f.count);
            if(!status) {goto done;}
            for(i=0;i<testmesspacked_v->test_enum_f.count;i++) {
                TestEnum* tmp;
                status = TestEnum_read(rt,&tmp);
                status = ast_append(rt,ast_enum,&testmesspacked_v->test_enum_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmesspacked_v->test_int32_f.exists) {
    testmesspacked_v->test_int32_f.value = 0;
}
if(testmesspacked_v->test_sint32_f.exists) {
    testmesspacked_v->test_sint32_f.value = 0;
}
if(testmesspacked_v->test_sfixed32_f.exists) {
    testmesspacked_v->test_sfixed32_f.value = 0;
}
if(testmesspacked_v->test_int64_f.exists) {
    testmesspacked_v->test_int64_f.value = 0;
}
if(testmesspacked_v->test_sint64_f.exists) {
    testmesspacked_v->test_sint64_f.value = 0;
}
if(testmesspacked_v->test_sfixed64_f.exists) {
    testmesspacked_v->test_sfixed64_f.value = 0;
}
if(testmesspacked_v->test_uint32_f.exists) {
    testmesspacked_v->test_uint32_f.value = 0;
}
if(testmesspacked_v->test_fixed32_f.exists) {
    testmesspacked_v->test_fixed32_f.value = 0;
}
if(testmesspacked_v->test_uint64_f.exists) {
    testmesspacked_v->test_uint64_f.value = 0;
}
if(testmesspacked_v->test_fixed64_f.exists) {
    testmesspacked_v->test_fixed64_f.value = 0;
}
if(testmesspacked_v->test_float_f.exists) {
    testmesspacked_v->test_float_f.value = 0;
}
if(testmesspacked_v->test_double_f.exists) {
    testmesspacked_v->test_double_f.value = 0;
}
if(testmesspacked_v->test_boolean_f.exists) {
    testmesspacked_v->test_boolean_f.value = 0;
}
done:
    return status;
} /*TestMessPacked_read*/

int
TestMessPacked_reclaim(ast_runtime* rt, TestMessPacked* testmesspacked_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessPacked_reclaim*/

long
TestMessPacked_size(ast_runtime* rt, TestMessPacked* testmesspacked_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    for(i=0;i<testmesspacked_v->test_int32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_int32,testmesspacked_v->test_int32_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_sint32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sint32,testmesspacked_v->test_sint32_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_sfixed32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sfixed32,testmesspacked_v->test_sfixed32_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_int64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_int64,testmesspacked_v->test_int64_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_sint64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sint64,testmesspacked_v->test_sint64_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_sfixed64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_sfixed64,testmesspacked_v->test_sfixed64_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_uint32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_uint32,testmesspacked_v->test_uint32_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_fixed32_f.count;i++) {
        totalsize += ast_write_size(rt,ast_fixed32,testmesspacked_v->test_fixed32_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_uint64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_uint64,testmesspacked_v->test_uint64_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_fixed64_f.count;i++) {
        totalsize += ast_write_size(rt,ast_fixed64,testmesspacked_v->test_fixed64_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_float_f.count;i++) {
        totalsize += ast_write_size(rt,ast_float,testmesspacked_v->test_float_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_double_f.count;i++) {
        totalsize += ast_write_size(rt,ast_double,testmesspacked_v->test_double_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_boolean_f.count;i++) {
        totalsize += ast_write_size(rt,ast_bool,testmesspacked_v->test_boolean_f.values[i]);
    }
    for(i=0;i<testmesspacked_v->test_enum_small_f.count;i++) {
        totalsize += TestEnumSmall_write_size(rt,14,&testmesspacked_v->test_enum_small_f);
    }
    for(i=0;i<testmesspacked_v->test_enum_f.count;i++) {
        totalsize += TestEnum_write_size(rt,15,&testmesspacked_v->test_enum_f);
    }
    return status;

} /*TestMessPacked_write_size*/
int
TestMessOptional_write(ast_runtime* rt, TestMessOptional* testmessoptional_v)
{
    int status = AST_NOERR;
    int i = 0;

    if(testmessoptional_v->test_int32_f.exists) {
        status = ast_write(rt,ast_int32,1,&testmessoptional_v->test_int32_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_sint32_f.exists) {
        status = ast_write(rt,ast_sint32,2,&testmessoptional_v->test_sint32_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_sfixed32_f.exists) {
        status = ast_write(rt,ast_sfixed32,3,&testmessoptional_v->test_sfixed32_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_int64_f.exists) {
        status = ast_write(rt,ast_int64,4,&testmessoptional_v->test_int64_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_sint64_f.exists) {
        status = ast_write(rt,ast_sint64,5,&testmessoptional_v->test_sint64_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_sfixed64_f.exists) {
        status = ast_write(rt,ast_sfixed64,6,&testmessoptional_v->test_sfixed64_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_uint32_f.exists) {
        status = ast_write(rt,ast_uint32,7,&testmessoptional_v->test_uint32_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_fixed32_f.exists) {
        status = ast_write(rt,ast_fixed32,8,&testmessoptional_v->test_fixed32_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_uint64_f.exists) {
        status = ast_write(rt,ast_uint64,9,&testmessoptional_v->test_uint64_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_fixed64_f.exists) {
        status = ast_write(rt,ast_fixed64,10,&testmessoptional_v->test_fixed64_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_float_f.exists) {
        status = ast_write(rt,ast_float,11,&testmessoptional_v->test_float_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_double_f.exists) {
        status = ast_write(rt,ast_double,12,&testmessoptional_v->test_double_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_boolean_f.exists) {
        status = ast_write(rt,ast_bool,13,&testmessoptional_v->test_boolean_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_enum_small_f.exists) {
        status = TestEnumSmall_write(rt,testmessoptional_v->test_enum_small_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_enum_f.exists) {
        status = TestEnum_write(rt,testmessoptional_v->test_enum_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_string_f.exists) {
        status = ast_write(rt,ast_string,16,&testmessoptional_v->test_string_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_bytes_f.exists) {
        status = ast_write(rt,ast_bytes,17,&testmessoptional_v->test_bytes_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_message_f.exists) {
        status = SubMess_write(rt,testmessoptional_v->test_message_f.value);
        if(!status) {goto done;}
    }

done:
    return status;

} /*TestMessOptional_write*/

int
TestMessOptional_read(ast_runtime* rt, TestMessOptional** testmessoptional_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    int i = 0;
    size_t count;
    TestMessOptional* testmessoptional_v;

    testmessoptional_v = (TestMessOptional*)ast_alloc(sizeof(TestMessOptional));
    if(testmessoptional_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            testmessoptional_v->test_int32_f.exists = 1;
            testmessoptional_v->test_int32_f.value = 0;
            status = ast_read(rt,ast_int32,&testmessoptional_v->test_int32_f.value);
            } break;
        case 2: {
            testmessoptional_v->test_sint32_f.exists = 1;
            testmessoptional_v->test_sint32_f.value = 0;
            status = ast_read(rt,ast_sint32,&testmessoptional_v->test_sint32_f.value);
            } break;
        case 3: {
            testmessoptional_v->test_sfixed32_f.exists = 1;
            testmessoptional_v->test_sfixed32_f.value = 0;
            status = ast_read(rt,ast_sfixed32,&testmessoptional_v->test_sfixed32_f.value);
            } break;
        case 4: {
            testmessoptional_v->test_int64_f.exists = 1;
            testmessoptional_v->test_int64_f.value = 0;
            status = ast_read(rt,ast_int64,&testmessoptional_v->test_int64_f.value);
            } break;
        case 5: {
            testmessoptional_v->test_sint64_f.exists = 1;
            testmessoptional_v->test_sint64_f.value = 0;
            status = ast_read(rt,ast_sint64,&testmessoptional_v->test_sint64_f.value);
            } break;
        case 6: {
            testmessoptional_v->test_sfixed64_f.exists = 1;
            testmessoptional_v->test_sfixed64_f.value = 0;
            status = ast_read(rt,ast_sfixed64,&testmessoptional_v->test_sfixed64_f.value);
            } break;
        case 7: {
            testmessoptional_v->test_uint32_f.exists = 1;
            testmessoptional_v->test_uint32_f.value = 0;
            status = ast_read(rt,ast_uint32,&testmessoptional_v->test_uint32_f.value);
            } break;
        case 8: {
            testmessoptional_v->test_fixed32_f.exists = 1;
            testmessoptional_v->test_fixed32_f.value = 0;
            status = ast_read(rt,ast_fixed32,&testmessoptional_v->test_fixed32_f.value);
            } break;
        case 9: {
            testmessoptional_v->test_uint64_f.exists = 1;
            testmessoptional_v->test_uint64_f.value = 0;
            status = ast_read(rt,ast_uint64,&testmessoptional_v->test_uint64_f.value);
            } break;
        case 10: {
            testmessoptional_v->test_fixed64_f.exists = 1;
            testmessoptional_v->test_fixed64_f.value = 0;
            status = ast_read(rt,ast_fixed64,&testmessoptional_v->test_fixed64_f.value);
            } break;
        case 11: {
            testmessoptional_v->test_float_f.exists = 1;
            testmessoptional_v->test_float_f.value = 0;
            status = ast_read(rt,ast_float,&testmessoptional_v->test_float_f.value);
            } break;
        case 12: {
            testmessoptional_v->test_double_f.exists = 1;
            testmessoptional_v->test_double_f.value = 0;
            status = ast_read(rt,ast_double,&testmessoptional_v->test_double_f.value);
            } break;
        case 13: {
            testmessoptional_v->test_boolean_f.exists = 1;
            testmessoptional_v->test_boolean_f.value = 0;
            status = ast_read(rt,ast_bool,&testmessoptional_v->test_boolean_f.value);
            } break;
        case 14: {
            testmessoptional_v->test_enum_small_f.exists = 1;
            testmessoptional_v->test_enum_small_f.value = NULL;
            status = TestEnumSmall_read(rt,&testmessoptional_v->test_enum_small_f.value);
            if(!status) {goto done;}
            } break;
        case 15: {
            testmessoptional_v->test_enum_f.exists = 1;
            testmessoptional_v->test_enum_f.value = NULL;
            status = TestEnum_read(rt,&testmessoptional_v->test_enum_f.value);
            if(!status) {goto done;}
            } break;
        case 16: {
            testmessoptional_v->test_string_f.exists = 1;
            testmessoptional_v->test_string_f.value = NULL;
            status = ast_read(rt,ast_string,&testmessoptional_v->test_string_f.value);
            } break;
        case 17: {
            testmessoptional_v->test_bytes_f.exists = 1;
            testmessoptional_v->test_bytes_f.value.nbytes = 0;
            testmessoptional_v->test_bytes_f.value.bytes = NULL;
            status = ast_read(rt,ast_bytes,&testmessoptional_v->test_bytes_f.value);
            } break;
        case 18: {
            testmessoptional_v->test_message_f.exists = 1;
            testmessoptional_v->test_message_f.value = NULL;
            status = SubMess_read(rt,&testmessoptional_v->test_message_f.value);
            if(!status) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessoptional_v->test_int32_f.exists) {
    testmessoptional_v->test_int32_f.value = 0;
}
if(testmessoptional_v->test_sint32_f.exists) {
    testmessoptional_v->test_sint32_f.value = 0;
}
if(testmessoptional_v->test_sfixed32_f.exists) {
    testmessoptional_v->test_sfixed32_f.value = 0;
}
if(testmessoptional_v->test_int64_f.exists) {
    testmessoptional_v->test_int64_f.value = 0;
}
if(testmessoptional_v->test_sint64_f.exists) {
    testmessoptional_v->test_sint64_f.value = 0;
}
if(testmessoptional_v->test_sfixed64_f.exists) {
    testmessoptional_v->test_sfixed64_f.value = 0;
}
if(testmessoptional_v->test_uint32_f.exists) {
    testmessoptional_v->test_uint32_f.value = 0;
}
if(testmessoptional_v->test_fixed32_f.exists) {
    testmessoptional_v->test_fixed32_f.value = 0;
}
if(testmessoptional_v->test_uint64_f.exists) {
    testmessoptional_v->test_uint64_f.value = 0;
}
if(testmessoptional_v->test_fixed64_f.exists) {
    testmessoptional_v->test_fixed64_f.value = 0;
}
if(testmessoptional_v->test_float_f.exists) {
    testmessoptional_v->test_float_f.value = 0;
}
if(testmessoptional_v->test_double_f.exists) {
    testmessoptional_v->test_double_f.value = 0;
}
if(testmessoptional_v->test_boolean_f.exists) {
    testmessoptional_v->test_boolean_f.value = 0;
}
done:
    return status;
} /*TestMessOptional_read*/

int
TestMessOptional_reclaim(ast_runtime* rt, TestMessOptional* testmessoptional_v)
{
    int status = AST_NOERR;
    int i = 0;

    if(testmessoptional_v->test_string_f.exists) {
        status = ast_reclaim(rt,ast_string,&testmessoptional_v->test_string_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_bytes_f.exists) {
        status = ast_reclaim(rt,ast_bytes,&testmessoptional_v->test_bytes_f.value);
        if(!status) {goto done;}
    }
    if(testmessoptional_v->test_message_f.exists) {
        status = SubMess_reclaim(rt,testmessoptional_v->test_message_f.value);
        if(!status) {goto done;}
    }

done:
    return status;

} /*TestMessOptional_reclaim*/

long
TestMessOptional_size(ast_runtime* rt, TestMessOptional* testmessoptional_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    if(testmessoptional_v->test_int32_f.exists) {
        totalsize += ast_write_size(rt,ast_int32,&testmessoptional_v->test_int32_f.value);
    }
    if(testmessoptional_v->test_sint32_f.exists) {
        totalsize += ast_write_size(rt,ast_sint32,&testmessoptional_v->test_sint32_f.value);
    }
    if(testmessoptional_v->test_sfixed32_f.exists) {
        totalsize += ast_write_size(rt,ast_sfixed32,&testmessoptional_v->test_sfixed32_f.value);
    }
    if(testmessoptional_v->test_int64_f.exists) {
        totalsize += ast_write_size(rt,ast_int64,&testmessoptional_v->test_int64_f.value);
    }
    if(testmessoptional_v->test_sint64_f.exists) {
        totalsize += ast_write_size(rt,ast_sint64,&testmessoptional_v->test_sint64_f.value);
    }
    if(testmessoptional_v->test_sfixed64_f.exists) {
        totalsize += ast_write_size(rt,ast_sfixed64,&testmessoptional_v->test_sfixed64_f.value);
    }
    if(testmessoptional_v->test_uint32_f.exists) {
        totalsize += ast_write_size(rt,ast_uint32,&testmessoptional_v->test_uint32_f.value);
    }
    if(testmessoptional_v->test_fixed32_f.exists) {
        totalsize += ast_write_size(rt,ast_fixed32,&testmessoptional_v->test_fixed32_f.value);
    }
    if(testmessoptional_v->test_uint64_f.exists) {
        totalsize += ast_write_size(rt,ast_uint64,&testmessoptional_v->test_uint64_f.value);
    }
    if(testmessoptional_v->test_fixed64_f.exists) {
        totalsize += ast_write_size(rt,ast_fixed64,&testmessoptional_v->test_fixed64_f.value);
    }
    if(testmessoptional_v->test_float_f.exists) {
        totalsize += ast_write_size(rt,ast_float,&testmessoptional_v->test_float_f.value);
    }
    if(testmessoptional_v->test_double_f.exists) {
        totalsize += ast_write_size(rt,ast_double,&testmessoptional_v->test_double_f.value);
    }
    if(testmessoptional_v->test_boolean_f.exists) {
        totalsize += ast_write_size(rt,ast_bool,&testmessoptional_v->test_boolean_f.value);
    }
    if(testmessoptional_v->test_enum_small_f.exists) {
        totalsize += TestEnumSmall_write_size(rt,14,testmessoptional_v->test_enum_small_f);
    }
    if(testmessoptional_v->test_enum_f.exists) {
        totalsize += TestEnum_write_size(rt,15,testmessoptional_v->test_enum_f);
    }
    if(testmessoptional_v->test_string_f.exists) {
        totalsize += ast_write_size(rt,ast_string,&testmessoptional_v->test_string_f.value);
    }
    if(testmessoptional_v->test_bytes_f.exists) {
        totalsize += ast_write_size(rt,ast_bytes,&testmessoptional_v->test_bytes_f.value);
    }
    if(testmessoptional_v->test_message_f.exists) {
        totalsize += SubMess_write_size(rt,18,testmessoptional_v->test_message_f);
    }
    return status;

} /*TestMessOptional_write_size*/
int
TestMessRequiredInt32_write(ast_runtime* rt, TestMessRequiredInt32* testmessrequiredint32_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_int32,42,&testmessrequiredint32_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredInt32_write*/

int
TestMessRequiredInt32_read(ast_runtime* rt, TestMessRequiredInt32** testmessrequiredint32_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredInt32* testmessrequiredint32_v;

    testmessrequiredint32_v = (TestMessRequiredInt32*)ast_alloc(sizeof(TestMessRequiredInt32));
    if(testmessrequiredint32_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 42: {
            status = ast_read(rt,ast_int32,&testmessrequiredint32_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredint32_v->test_f.exists) {
    testmessrequiredint32_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredInt32_read*/

int
TestMessRequiredInt32_reclaim(ast_runtime* rt, TestMessRequiredInt32* testmessrequiredint32_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredInt32_reclaim*/

long
TestMessRequiredInt32_size(ast_runtime* rt, TestMessRequiredInt32* testmessrequiredint32_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_int32,&testmessrequiredint32_v->test_f);
    return status;

} /*TestMessRequiredInt32_write_size*/
int
TestMessRequiredSInt32_write(ast_runtime* rt, TestMessRequiredSInt32* testmessrequiredsint32_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_sint32,43,&testmessrequiredsint32_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredSInt32_write*/

int
TestMessRequiredSInt32_read(ast_runtime* rt, TestMessRequiredSInt32** testmessrequiredsint32_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredSInt32* testmessrequiredsint32_v;

    testmessrequiredsint32_v = (TestMessRequiredSInt32*)ast_alloc(sizeof(TestMessRequiredSInt32));
    if(testmessrequiredsint32_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 43: {
            status = ast_read(rt,ast_sint32,&testmessrequiredsint32_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredsint32_v->test_f.exists) {
    testmessrequiredsint32_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredSInt32_read*/

int
TestMessRequiredSInt32_reclaim(ast_runtime* rt, TestMessRequiredSInt32* testmessrequiredsint32_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredSInt32_reclaim*/

long
TestMessRequiredSInt32_size(ast_runtime* rt, TestMessRequiredSInt32* testmessrequiredsint32_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_sint32,&testmessrequiredsint32_v->test_f);
    return status;

} /*TestMessRequiredSInt32_write_size*/
int
TestMessRequiredSFixed32_write(ast_runtime* rt, TestMessRequiredSFixed32* testmessrequiredsfixed32_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_sfixed32,100,&testmessrequiredsfixed32_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredSFixed32_write*/

int
TestMessRequiredSFixed32_read(ast_runtime* rt, TestMessRequiredSFixed32** testmessrequiredsfixed32_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredSFixed32* testmessrequiredsfixed32_v;

    testmessrequiredsfixed32_v = (TestMessRequiredSFixed32*)ast_alloc(sizeof(TestMessRequiredSFixed32));
    if(testmessrequiredsfixed32_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 100: {
            status = ast_read(rt,ast_sfixed32,&testmessrequiredsfixed32_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredsfixed32_v->test_f.exists) {
    testmessrequiredsfixed32_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredSFixed32_read*/

int
TestMessRequiredSFixed32_reclaim(ast_runtime* rt, TestMessRequiredSFixed32* testmessrequiredsfixed32_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredSFixed32_reclaim*/

long
TestMessRequiredSFixed32_size(ast_runtime* rt, TestMessRequiredSFixed32* testmessrequiredsfixed32_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_sfixed32,&testmessrequiredsfixed32_v->test_f);
    return status;

} /*TestMessRequiredSFixed32_write_size*/
int
TestMessRequiredInt64_write(ast_runtime* rt, TestMessRequiredInt64* testmessrequiredint64_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_int64,1,&testmessrequiredint64_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredInt64_write*/

int
TestMessRequiredInt64_read(ast_runtime* rt, TestMessRequiredInt64** testmessrequiredint64_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredInt64* testmessrequiredint64_v;

    testmessrequiredint64_v = (TestMessRequiredInt64*)ast_alloc(sizeof(TestMessRequiredInt64));
    if(testmessrequiredint64_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_int64,&testmessrequiredint64_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredint64_v->test_f.exists) {
    testmessrequiredint64_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredInt64_read*/

int
TestMessRequiredInt64_reclaim(ast_runtime* rt, TestMessRequiredInt64* testmessrequiredint64_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredInt64_reclaim*/

long
TestMessRequiredInt64_size(ast_runtime* rt, TestMessRequiredInt64* testmessrequiredint64_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_int64,&testmessrequiredint64_v->test_f);
    return status;

} /*TestMessRequiredInt64_write_size*/
int
TestMessRequiredSInt64_write(ast_runtime* rt, TestMessRequiredSInt64* testmessrequiredsint64_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_sint64,11,&testmessrequiredsint64_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredSInt64_write*/

int
TestMessRequiredSInt64_read(ast_runtime* rt, TestMessRequiredSInt64** testmessrequiredsint64_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredSInt64* testmessrequiredsint64_v;

    testmessrequiredsint64_v = (TestMessRequiredSInt64*)ast_alloc(sizeof(TestMessRequiredSInt64));
    if(testmessrequiredsint64_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 11: {
            status = ast_read(rt,ast_sint64,&testmessrequiredsint64_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredsint64_v->test_f.exists) {
    testmessrequiredsint64_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredSInt64_read*/

int
TestMessRequiredSInt64_reclaim(ast_runtime* rt, TestMessRequiredSInt64* testmessrequiredsint64_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredSInt64_reclaim*/

long
TestMessRequiredSInt64_size(ast_runtime* rt, TestMessRequiredSInt64* testmessrequiredsint64_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_sint64,&testmessrequiredsint64_v->test_f);
    return status;

} /*TestMessRequiredSInt64_write_size*/
int
TestMessRequiredSFixed64_write(ast_runtime* rt, TestMessRequiredSFixed64* testmessrequiredsfixed64_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_sfixed64,12,&testmessrequiredsfixed64_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredSFixed64_write*/

int
TestMessRequiredSFixed64_read(ast_runtime* rt, TestMessRequiredSFixed64** testmessrequiredsfixed64_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredSFixed64* testmessrequiredsfixed64_v;

    testmessrequiredsfixed64_v = (TestMessRequiredSFixed64*)ast_alloc(sizeof(TestMessRequiredSFixed64));
    if(testmessrequiredsfixed64_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 12: {
            status = ast_read(rt,ast_sfixed64,&testmessrequiredsfixed64_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredsfixed64_v->test_f.exists) {
    testmessrequiredsfixed64_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredSFixed64_read*/

int
TestMessRequiredSFixed64_reclaim(ast_runtime* rt, TestMessRequiredSFixed64* testmessrequiredsfixed64_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredSFixed64_reclaim*/

long
TestMessRequiredSFixed64_size(ast_runtime* rt, TestMessRequiredSFixed64* testmessrequiredsfixed64_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_sfixed64,&testmessrequiredsfixed64_v->test_f);
    return status;

} /*TestMessRequiredSFixed64_write_size*/
int
TestMessRequiredUInt32_write(ast_runtime* rt, TestMessRequiredUInt32* testmessrequireduint32_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_uint32,1,&testmessrequireduint32_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredUInt32_write*/

int
TestMessRequiredUInt32_read(ast_runtime* rt, TestMessRequiredUInt32** testmessrequireduint32_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredUInt32* testmessrequireduint32_v;

    testmessrequireduint32_v = (TestMessRequiredUInt32*)ast_alloc(sizeof(TestMessRequiredUInt32));
    if(testmessrequireduint32_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_uint32,&testmessrequireduint32_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequireduint32_v->test_f.exists) {
    testmessrequireduint32_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredUInt32_read*/

int
TestMessRequiredUInt32_reclaim(ast_runtime* rt, TestMessRequiredUInt32* testmessrequireduint32_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredUInt32_reclaim*/

long
TestMessRequiredUInt32_size(ast_runtime* rt, TestMessRequiredUInt32* testmessrequireduint32_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_uint32,&testmessrequireduint32_v->test_f);
    return status;

} /*TestMessRequiredUInt32_write_size*/
int
TestMessRequiredFixed32_write(ast_runtime* rt, TestMessRequiredFixed32* testmessrequiredfixed32_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_fixed32,1,&testmessrequiredfixed32_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredFixed32_write*/

int
TestMessRequiredFixed32_read(ast_runtime* rt, TestMessRequiredFixed32** testmessrequiredfixed32_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredFixed32* testmessrequiredfixed32_v;

    testmessrequiredfixed32_v = (TestMessRequiredFixed32*)ast_alloc(sizeof(TestMessRequiredFixed32));
    if(testmessrequiredfixed32_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_fixed32,&testmessrequiredfixed32_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredfixed32_v->test_f.exists) {
    testmessrequiredfixed32_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredFixed32_read*/

int
TestMessRequiredFixed32_reclaim(ast_runtime* rt, TestMessRequiredFixed32* testmessrequiredfixed32_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredFixed32_reclaim*/

long
TestMessRequiredFixed32_size(ast_runtime* rt, TestMessRequiredFixed32* testmessrequiredfixed32_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_fixed32,&testmessrequiredfixed32_v->test_f);
    return status;

} /*TestMessRequiredFixed32_write_size*/
int
TestMessRequiredUInt64_write(ast_runtime* rt, TestMessRequiredUInt64* testmessrequireduint64_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_uint64,1,&testmessrequireduint64_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredUInt64_write*/

int
TestMessRequiredUInt64_read(ast_runtime* rt, TestMessRequiredUInt64** testmessrequireduint64_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredUInt64* testmessrequireduint64_v;

    testmessrequireduint64_v = (TestMessRequiredUInt64*)ast_alloc(sizeof(TestMessRequiredUInt64));
    if(testmessrequireduint64_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_uint64,&testmessrequireduint64_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequireduint64_v->test_f.exists) {
    testmessrequireduint64_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredUInt64_read*/

int
TestMessRequiredUInt64_reclaim(ast_runtime* rt, TestMessRequiredUInt64* testmessrequireduint64_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredUInt64_reclaim*/

long
TestMessRequiredUInt64_size(ast_runtime* rt, TestMessRequiredUInt64* testmessrequireduint64_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_uint64,&testmessrequireduint64_v->test_f);
    return status;

} /*TestMessRequiredUInt64_write_size*/
int
TestMessRequiredFixed64_write(ast_runtime* rt, TestMessRequiredFixed64* testmessrequiredfixed64_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_fixed64,1,&testmessrequiredfixed64_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredFixed64_write*/

int
TestMessRequiredFixed64_read(ast_runtime* rt, TestMessRequiredFixed64** testmessrequiredfixed64_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredFixed64* testmessrequiredfixed64_v;

    testmessrequiredfixed64_v = (TestMessRequiredFixed64*)ast_alloc(sizeof(TestMessRequiredFixed64));
    if(testmessrequiredfixed64_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_fixed64,&testmessrequiredfixed64_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredfixed64_v->test_f.exists) {
    testmessrequiredfixed64_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredFixed64_read*/

int
TestMessRequiredFixed64_reclaim(ast_runtime* rt, TestMessRequiredFixed64* testmessrequiredfixed64_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredFixed64_reclaim*/

long
TestMessRequiredFixed64_size(ast_runtime* rt, TestMessRequiredFixed64* testmessrequiredfixed64_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_fixed64,&testmessrequiredfixed64_v->test_f);
    return status;

} /*TestMessRequiredFixed64_write_size*/
int
TestMessRequiredFloat_write(ast_runtime* rt, TestMessRequiredFloat* testmessrequiredfloat_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_float,1,&testmessrequiredfloat_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredFloat_write*/

int
TestMessRequiredFloat_read(ast_runtime* rt, TestMessRequiredFloat** testmessrequiredfloat_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredFloat* testmessrequiredfloat_v;

    testmessrequiredfloat_v = (TestMessRequiredFloat*)ast_alloc(sizeof(TestMessRequiredFloat));
    if(testmessrequiredfloat_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_float,&testmessrequiredfloat_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredfloat_v->test_f.exists) {
    testmessrequiredfloat_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredFloat_read*/

int
TestMessRequiredFloat_reclaim(ast_runtime* rt, TestMessRequiredFloat* testmessrequiredfloat_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredFloat_reclaim*/

long
TestMessRequiredFloat_size(ast_runtime* rt, TestMessRequiredFloat* testmessrequiredfloat_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_float,&testmessrequiredfloat_v->test_f);
    return status;

} /*TestMessRequiredFloat_write_size*/
int
TestMessRequiredDouble_write(ast_runtime* rt, TestMessRequiredDouble* testmessrequireddouble_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_double,1,&testmessrequireddouble_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredDouble_write*/

int
TestMessRequiredDouble_read(ast_runtime* rt, TestMessRequiredDouble** testmessrequireddouble_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredDouble* testmessrequireddouble_v;

    testmessrequireddouble_v = (TestMessRequiredDouble*)ast_alloc(sizeof(TestMessRequiredDouble));
    if(testmessrequireddouble_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_double,&testmessrequireddouble_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequireddouble_v->test_f.exists) {
    testmessrequireddouble_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredDouble_read*/

int
TestMessRequiredDouble_reclaim(ast_runtime* rt, TestMessRequiredDouble* testmessrequireddouble_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredDouble_reclaim*/

long
TestMessRequiredDouble_size(ast_runtime* rt, TestMessRequiredDouble* testmessrequireddouble_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_double,&testmessrequireddouble_v->test_f);
    return status;

} /*TestMessRequiredDouble_write_size*/
int
TestMessRequiredBool_write(ast_runtime* rt, TestMessRequiredBool* testmessrequiredbool_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_bool,1,&testmessrequiredbool_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredBool_write*/

int
TestMessRequiredBool_read(ast_runtime* rt, TestMessRequiredBool** testmessrequiredbool_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredBool* testmessrequiredbool_v;

    testmessrequiredbool_v = (TestMessRequiredBool*)ast_alloc(sizeof(TestMessRequiredBool));
    if(testmessrequiredbool_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_bool,&testmessrequiredbool_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(testmessrequiredbool_v->test_f.exists) {
    testmessrequiredbool_v->test_f.value = 0;
}
done:
    return status;
} /*TestMessRequiredBool_read*/

int
TestMessRequiredBool_reclaim(ast_runtime* rt, TestMessRequiredBool* testmessrequiredbool_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredBool_reclaim*/

long
TestMessRequiredBool_size(ast_runtime* rt, TestMessRequiredBool* testmessrequiredbool_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_bool,&testmessrequiredbool_v->test_f);
    return status;

} /*TestMessRequiredBool_write_size*/
int
TestMessRequiredEnum_write(ast_runtime* rt, TestMessRequiredEnum* testmessrequiredenum_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = TestEnum_write(rt,testmessrequiredenum_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredEnum_write*/

int
TestMessRequiredEnum_read(ast_runtime* rt, TestMessRequiredEnum** testmessrequiredenum_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredEnum* testmessrequiredenum_v;

    testmessrequiredenum_v = (TestMessRequiredEnum*)ast_alloc(sizeof(TestMessRequiredEnum));
    if(testmessrequiredenum_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = TestEnum_read(rt,&testmessrequiredenum_v->test_f);
            if(!status) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestMessRequiredEnum_read*/

int
TestMessRequiredEnum_reclaim(ast_runtime* rt, TestMessRequiredEnum* testmessrequiredenum_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredEnum_reclaim*/

long
TestMessRequiredEnum_size(ast_runtime* rt, TestMessRequiredEnum* testmessrequiredenum_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += TestEnum_write_size(rt,1,testmessrequiredenum_v->test_f);
    return status;

} /*TestMessRequiredEnum_write_size*/
int
TestMessRequiredEnumSmall_write(ast_runtime* rt, TestMessRequiredEnumSmall* testmessrequiredenumsmall_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = TestEnumSmall_write(rt,testmessrequiredenumsmall_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredEnumSmall_write*/

int
TestMessRequiredEnumSmall_read(ast_runtime* rt, TestMessRequiredEnumSmall** testmessrequiredenumsmall_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredEnumSmall* testmessrequiredenumsmall_v;

    testmessrequiredenumsmall_v = (TestMessRequiredEnumSmall*)ast_alloc(sizeof(TestMessRequiredEnumSmall));
    if(testmessrequiredenumsmall_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = TestEnumSmall_read(rt,&testmessrequiredenumsmall_v->test_f);
            if(!status) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestMessRequiredEnumSmall_read*/

int
TestMessRequiredEnumSmall_reclaim(ast_runtime* rt, TestMessRequiredEnumSmall* testmessrequiredenumsmall_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*TestMessRequiredEnumSmall_reclaim*/

long
TestMessRequiredEnumSmall_size(ast_runtime* rt, TestMessRequiredEnumSmall* testmessrequiredenumsmall_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += TestEnumSmall_write_size(rt,1,testmessrequiredenumsmall_v->test_f);
    return status;

} /*TestMessRequiredEnumSmall_write_size*/
int
TestMessRequiredString_write(ast_runtime* rt, TestMessRequiredString* testmessrequiredstring_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_string,1,&testmessrequiredstring_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredString_write*/

int
TestMessRequiredString_read(ast_runtime* rt, TestMessRequiredString** testmessrequiredstring_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestMessRequiredString* testmessrequiredstring_v;

    testmessrequiredstring_v = (TestMessRequiredString*)ast_alloc(sizeof(TestMessRequiredString));
    if(testmessrequiredstring_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_string,&testmessrequiredstring_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestMessRequiredString_read*/

int
TestMessRequiredString_reclaim(ast_runtime* rt, TestMessRequiredString* testmessrequiredstring_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&testmessrequiredstring_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredString_reclaim*/

long
TestMessRequiredString_size(ast_runtime* rt, TestMessRequiredString* testmessrequiredstring_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_string,&testmessrequiredstring_v->test_f);
    return status;

} /*TestMessRequiredString_write_size*/
int
TestMessRequiredBytes_write(ast_runtime* rt, TestMessRequiredBytes* testmessrequiredbytes_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_bytes,1,&testmessrequiredbytes_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredBytes_write*/

int
TestMessRequiredBytes_read(ast_runtime* rt, TestMessRequiredBytes** testmessrequiredbytes_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    TestMessRequiredBytes* testmessrequiredbytes_v;

    testmessrequiredbytes_v = (TestMessRequiredBytes*)ast_alloc(sizeof(TestMessRequiredBytes));
    if(testmessrequiredbytes_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_bytes,&testmessrequiredbytes_v->test_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestMessRequiredBytes_read*/

int
TestMessRequiredBytes_reclaim(ast_runtime* rt, TestMessRequiredBytes* testmessrequiredbytes_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_bytes,&testmessrequiredbytes_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredBytes_reclaim*/

long
TestMessRequiredBytes_size(ast_runtime* rt, TestMessRequiredBytes* testmessrequiredbytes_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_bytes,&testmessrequiredbytes_v->test_f);
    return status;

} /*TestMessRequiredBytes_write_size*/
int
TestMessRequiredMessage_write(ast_runtime* rt, TestMessRequiredMessage* testmessrequiredmessage_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = SubMess_write(rt,testmessrequiredmessage_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredMessage_write*/

int
TestMessRequiredMessage_read(ast_runtime* rt, TestMessRequiredMessage** testmessrequiredmessage_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    TestMessRequiredMessage* testmessrequiredmessage_v;

    testmessrequiredmessage_v = (TestMessRequiredMessage*)ast_alloc(sizeof(TestMessRequiredMessage));
    if(testmessrequiredmessage_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = SubMess_read(rt,&testmessrequiredmessage_v->test_f);
            if(!status) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*TestMessRequiredMessage_read*/

int
TestMessRequiredMessage_reclaim(ast_runtime* rt, TestMessRequiredMessage* testmessrequiredmessage_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = SubMess_reclaim(rt,testmessrequiredmessage_v->test_f);
    if(!status) {goto done;}

done:
    return status;

} /*TestMessRequiredMessage_reclaim*/

long
TestMessRequiredMessage_size(ast_runtime* rt, TestMessRequiredMessage* testmessrequiredmessage_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += SubMess_write_size(rt,1,testmessrequiredmessage_v->test_f);
    return status;

} /*TestMessRequiredMessage_write_size*/
int
EmptyMess_write(ast_runtime* rt, EmptyMess* emptymess_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*EmptyMess_write*/

int
EmptyMess_read(ast_runtime* rt, EmptyMess** emptymess_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    EmptyMess* emptymess_v;

    emptymess_v = (EmptyMess*)ast_alloc(sizeof(EmptyMess));
    if(emptymess_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*EmptyMess_read*/

int
EmptyMess_reclaim(ast_runtime* rt, EmptyMess* emptymess_v)
{
    int status = AST_NOERR;
    int i = 0;


done:
    return status;

} /*EmptyMess_reclaim*/

long
EmptyMess_size(ast_runtime* rt, EmptyMess* emptymess_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    return status;

} /*EmptyMess_write_size*/
int
DefaultRequiredValues_write(ast_runtime* rt, DefaultRequiredValues* defaultrequiredvalues_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_write(rt,ast_int32,1,&defaultrequiredvalues_v->v_int32_f);
    if(!status) {goto done;}
    status = ast_write(rt,ast_uint32,2,&defaultrequiredvalues_v->v_uint32_f);
    if(!status) {goto done;}
    status = ast_write(rt,ast_int32,3,&defaultrequiredvalues_v->v_int64_f);
    if(!status) {goto done;}
    status = ast_write(rt,ast_uint32,4,&defaultrequiredvalues_v->v_uint64_f);
    if(!status) {goto done;}
    status = ast_write(rt,ast_float,5,&defaultrequiredvalues_v->v_float_f);
    if(!status) {goto done;}
    status = ast_write(rt,ast_double,6,&defaultrequiredvalues_v->v_double_f);
    if(!status) {goto done;}
    status = ast_write(rt,ast_string,7,&defaultrequiredvalues_v->v_string_f);
    if(!status) {goto done;}
    status = ast_write(rt,ast_bytes,8,&defaultrequiredvalues_v->v_bytes_f);
    if(!status) {goto done;}

done:
    return status;

} /*DefaultRequiredValues_write*/

int
DefaultRequiredValues_read(ast_runtime* rt, DefaultRequiredValues** defaultrequiredvalues_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    size_t count;
    DefaultRequiredValues* defaultrequiredvalues_v;

    defaultrequiredvalues_v = (DefaultRequiredValues*)ast_alloc(sizeof(DefaultRequiredValues));
    if(defaultrequiredvalues_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            status = ast_read(rt,ast_int32,&defaultrequiredvalues_v->v_int32_f);
            } break;
        case 2: {
            status = ast_read(rt,ast_uint32,&defaultrequiredvalues_v->v_uint32_f);
            } break;
        case 3: {
            status = ast_read(rt,ast_int32,&defaultrequiredvalues_v->v_int64_f);
            } break;
        case 4: {
            status = ast_read(rt,ast_uint32,&defaultrequiredvalues_v->v_uint64_f);
            } break;
        case 5: {
            status = ast_read(rt,ast_float,&defaultrequiredvalues_v->v_float_f);
            } break;
        case 6: {
            status = ast_read(rt,ast_double,&defaultrequiredvalues_v->v_double_f);
            } break;
        case 7: {
            status = ast_read(rt,ast_string,&defaultrequiredvalues_v->v_string_f);
            } break;
        case 8: {
            status = ast_read(rt,ast_bytes,&defaultrequiredvalues_v->v_bytes_f);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(defaultrequiredvalues_v->v_int32_f.exists) {
    defaultrequiredvalues_v->v_int32_f.value = 0;
}
if(defaultrequiredvalues_v->v_uint32_f.exists) {
    defaultrequiredvalues_v->v_uint32_f.value = 0;
}
if(defaultrequiredvalues_v->v_int64_f.exists) {
    defaultrequiredvalues_v->v_int64_f.value = 0;
}
if(defaultrequiredvalues_v->v_uint64_f.exists) {
    defaultrequiredvalues_v->v_uint64_f.value = 0;
}
if(defaultrequiredvalues_v->v_float_f.exists) {
    defaultrequiredvalues_v->v_float_f.value = 0;
}
if(defaultrequiredvalues_v->v_double_f.exists) {
    defaultrequiredvalues_v->v_double_f.value = 0;
}
done:
    return status;
} /*DefaultRequiredValues_read*/

int
DefaultRequiredValues_reclaim(ast_runtime* rt, DefaultRequiredValues* defaultrequiredvalues_v)
{
    int status = AST_NOERR;
    int i = 0;

    status = ast_reclaim(rt,ast_string,&defaultrequiredvalues_v->v_string_f);
    if(!status) {goto done;}
    status = ast_reclaim(rt,ast_bytes,&defaultrequiredvalues_v->v_bytes_f);
    if(!status) {goto done;}

done:
    return status;

} /*DefaultRequiredValues_reclaim*/

long
DefaultRequiredValues_size(ast_runtime* rt, DefaultRequiredValues* defaultrequiredvalues_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    totalsize += ast_write_size(rt,ast_int32,&defaultrequiredvalues_v->v_int32_f);
    totalsize += ast_write_size(rt,ast_uint32,&defaultrequiredvalues_v->v_uint32_f);
    totalsize += ast_write_size(rt,ast_int32,&defaultrequiredvalues_v->v_int64_f);
    totalsize += ast_write_size(rt,ast_uint32,&defaultrequiredvalues_v->v_uint64_f);
    totalsize += ast_write_size(rt,ast_float,&defaultrequiredvalues_v->v_float_f);
    totalsize += ast_write_size(rt,ast_double,&defaultrequiredvalues_v->v_double_f);
    totalsize += ast_write_size(rt,ast_string,&defaultrequiredvalues_v->v_string_f);
    totalsize += ast_write_size(rt,ast_bytes,&defaultrequiredvalues_v->v_bytes_f);
    return status;

} /*DefaultRequiredValues_write_size*/
int
DefaultOptionalValues_write(ast_runtime* rt, DefaultOptionalValues* defaultoptionalvalues_v)
{
    int status = AST_NOERR;
    int i = 0;

    if(defaultoptionalvalues_v->v_int32_f.exists) {
        status = ast_write(rt,ast_int32,1,&defaultoptionalvalues_v->v_int32_f.value);
        if(!status) {goto done;}
    }
    if(defaultoptionalvalues_v->v_uint32_f.exists) {
        status = ast_write(rt,ast_uint32,2,&defaultoptionalvalues_v->v_uint32_f.value);
        if(!status) {goto done;}
    }
    if(defaultoptionalvalues_v->v_int64_f.exists) {
        status = ast_write(rt,ast_int32,3,&defaultoptionalvalues_v->v_int64_f.value);
        if(!status) {goto done;}
    }
    if(defaultoptionalvalues_v->v_uint64_f.exists) {
        status = ast_write(rt,ast_uint32,4,&defaultoptionalvalues_v->v_uint64_f.value);
        if(!status) {goto done;}
    }
    if(defaultoptionalvalues_v->v_float_f.exists) {
        status = ast_write(rt,ast_float,5,&defaultoptionalvalues_v->v_float_f.value);
        if(!status) {goto done;}
    }
    if(defaultoptionalvalues_v->v_double_f.exists) {
        status = ast_write(rt,ast_double,6,&defaultoptionalvalues_v->v_double_f.value);
        if(!status) {goto done;}
    }
    if(defaultoptionalvalues_v->v_string_f.exists) {
        status = ast_write(rt,ast_string,7,&defaultoptionalvalues_v->v_string_f.value);
        if(!status) {goto done;}
    }
    if(defaultoptionalvalues_v->v_bytes_f.exists) {
        status = ast_write(rt,ast_bytes,8,&defaultoptionalvalues_v->v_bytes_f.value);
        if(!status) {goto done;}
    }

done:
    return status;

} /*DefaultOptionalValues_write*/

int
DefaultOptionalValues_read(ast_runtime* rt, DefaultOptionalValues** defaultoptionalvalues_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    int i = 0;
    size_t count;
    DefaultOptionalValues* defaultoptionalvalues_v;

    defaultoptionalvalues_v = (DefaultOptionalValues*)ast_alloc(sizeof(DefaultOptionalValues));
    if(defaultoptionalvalues_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            defaultoptionalvalues_v->v_int32_f.exists = 1;
            defaultoptionalvalues_v->v_int32_f.value = 0;
            status = ast_read(rt,ast_int32,&defaultoptionalvalues_v->v_int32_f.value);
            } break;
        case 2: {
            defaultoptionalvalues_v->v_uint32_f.exists = 1;
            defaultoptionalvalues_v->v_uint32_f.value = 0;
            status = ast_read(rt,ast_uint32,&defaultoptionalvalues_v->v_uint32_f.value);
            } break;
        case 3: {
            defaultoptionalvalues_v->v_int64_f.exists = 1;
            defaultoptionalvalues_v->v_int64_f.value = 0;
            status = ast_read(rt,ast_int32,&defaultoptionalvalues_v->v_int64_f.value);
            } break;
        case 4: {
            defaultoptionalvalues_v->v_uint64_f.exists = 1;
            defaultoptionalvalues_v->v_uint64_f.value = 0;
            status = ast_read(rt,ast_uint32,&defaultoptionalvalues_v->v_uint64_f.value);
            } break;
        case 5: {
            defaultoptionalvalues_v->v_float_f.exists = 1;
            defaultoptionalvalues_v->v_float_f.value = 0;
            status = ast_read(rt,ast_float,&defaultoptionalvalues_v->v_float_f.value);
            } break;
        case 6: {
            defaultoptionalvalues_v->v_double_f.exists = 1;
            defaultoptionalvalues_v->v_double_f.value = 0;
            status = ast_read(rt,ast_double,&defaultoptionalvalues_v->v_double_f.value);
            } break;
        case 7: {
            defaultoptionalvalues_v->v_string_f.exists = 1;
            defaultoptionalvalues_v->v_string_f.value = NULL;
            status = ast_read(rt,ast_string,&defaultoptionalvalues_v->v_string_f.value);
            } break;
        case 8: {
            defaultoptionalvalues_v->v_bytes_f.exists = 1;
            defaultoptionalvalues_v->v_bytes_f.value.nbytes = 0;
            defaultoptionalvalues_v->v_bytes_f.value.bytes = NULL;
            status = ast_read(rt,ast_bytes,&defaultoptionalvalues_v->v_bytes_f.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
if(defaultoptionalvalues_v->v_int32_f.exists) {
    defaultoptionalvalues_v->v_int32_f.value = 0;
}
if(defaultoptionalvalues_v->v_uint32_f.exists) {
    defaultoptionalvalues_v->v_uint32_f.value = 0;
}
if(defaultoptionalvalues_v->v_int64_f.exists) {
    defaultoptionalvalues_v->v_int64_f.value = 0;
}
if(defaultoptionalvalues_v->v_uint64_f.exists) {
    defaultoptionalvalues_v->v_uint64_f.value = 0;
}
if(defaultoptionalvalues_v->v_float_f.exists) {
    defaultoptionalvalues_v->v_float_f.value = 0;
}
if(defaultoptionalvalues_v->v_double_f.exists) {
    defaultoptionalvalues_v->v_double_f.value = 0;
}
done:
    return status;
} /*DefaultOptionalValues_read*/

int
DefaultOptionalValues_reclaim(ast_runtime* rt, DefaultOptionalValues* defaultoptionalvalues_v)
{
    int status = AST_NOERR;
    int i = 0;

    if(defaultoptionalvalues_v->v_string_f.exists) {
        status = ast_reclaim(rt,ast_string,&defaultoptionalvalues_v->v_string_f.value);
        if(!status) {goto done;}
    }
    if(defaultoptionalvalues_v->v_bytes_f.exists) {
        status = ast_reclaim(rt,ast_bytes,&defaultoptionalvalues_v->v_bytes_f.value);
        if(!status) {goto done;}
    }

done:
    return status;

} /*DefaultOptionalValues_reclaim*/

long
DefaultOptionalValues_size(ast_runtime* rt, DefaultOptionalValues* defaultoptionalvalues_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    if(defaultoptionalvalues_v->v_int32_f.exists) {
        totalsize += ast_write_size(rt,ast_int32,&defaultoptionalvalues_v->v_int32_f.value);
    }
    if(defaultoptionalvalues_v->v_uint32_f.exists) {
        totalsize += ast_write_size(rt,ast_uint32,&defaultoptionalvalues_v->v_uint32_f.value);
    }
    if(defaultoptionalvalues_v->v_int64_f.exists) {
        totalsize += ast_write_size(rt,ast_int32,&defaultoptionalvalues_v->v_int64_f.value);
    }
    if(defaultoptionalvalues_v->v_uint64_f.exists) {
        totalsize += ast_write_size(rt,ast_uint32,&defaultoptionalvalues_v->v_uint64_f.value);
    }
    if(defaultoptionalvalues_v->v_float_f.exists) {
        totalsize += ast_write_size(rt,ast_float,&defaultoptionalvalues_v->v_float_f.value);
    }
    if(defaultoptionalvalues_v->v_double_f.exists) {
        totalsize += ast_write_size(rt,ast_double,&defaultoptionalvalues_v->v_double_f.value);
    }
    if(defaultoptionalvalues_v->v_string_f.exists) {
        totalsize += ast_write_size(rt,ast_string,&defaultoptionalvalues_v->v_string_f.value);
    }
    if(defaultoptionalvalues_v->v_bytes_f.exists) {
        totalsize += ast_write_size(rt,ast_bytes,&defaultoptionalvalues_v->v_bytes_f.value);
    }
    return status;

} /*DefaultOptionalValues_write_size*/
int
AllocValues_write(ast_runtime* rt, AllocValues* allocvalues_v)
{
    int status = AST_NOERR;
    int i = 0;

    if(allocvalues_v->o_bytes_f.exists) {
        status = ast_write(rt,ast_bytes,1,&allocvalues_v->o_bytes_f.value);
        if(!status) {goto done;}
    }
    for(i=0;i<allocvalues_v->r_string_f.count;i++) {
        status = ast_write(rt,ast_string,2,&allocvalues_v->r_string_f.values[i]);
        if(!status) {goto done;}
    }
    status = ast_write(rt,ast_string,3,&allocvalues_v->a_string_f);
    if(!status) {goto done;}
    status = ast_write(rt,ast_bytes,4,&allocvalues_v->a_bytes_f);
    if(!status) {goto done;}
    status = DefaultRequiredValues_write(rt,allocvalues_v->a_mess_f);
    if(!status) {goto done;}

done:
    return status;

} /*AllocValues_write*/

int
AllocValues_read(ast_runtime* rt, AllocValues** allocvalues_vp)
{
    int status = AST_NOERR;
    uint32_t wiretype, fieldno;
    int i = 0;
    size_t count;
    AllocValues* allocvalues_v;

    allocvalues_v = (AllocValues*)ast_alloc(sizeof(AllocValues));
    if(allocvalues_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR)
        ast_read_tag(rt,&wiretype,&fieldno)
        switch (fieldno) {
        case 1: {
            allocvalues_v->o_bytes_f.exists = 1;
            allocvalues_v->o_bytes_f.value.nbytes = 0;
            allocvalues_v->o_bytes_f.value.bytes = NULL;
            status = ast_read(rt,ast_bytes,&allocvalues_v->o_bytes_f.value);
            } break;
        case 2: {
            allocvalues_v->r_string_f.count = 0;
            allocvalues_v->r_string_f.values = NULL;
            status = ast_read_count(rt,&allocvalues_v->r_string_f.count);
            if(!status) {goto done;}
            for(i=0;i<allocvalues_v->r_string_f.count;i++) {
                char* tmp;
                status = ast_read(rt,ast_string,&tmp);
                status = ast_append(rt,ast_string,&allocvalues_v->r_string_f,&tmp);
                if(!status) {goto done;}
            } /*for*/;
            } break;
        case 3: {
            status = ast_read(rt,ast_string,&allocvalues_v->a_string_f);
            } break;
        case 4: {
            status = ast_read(rt,ast_bytes,&allocvalues_v->a_bytes_f);
            } break;
        case 5: {
            status = DefaultRequiredValues_read(rt,&allocvalues_v->a_mess_f);
            if(!status) {goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
        }; /*switch*/
    };/*while*/
done:
    return status;
} /*AllocValues_read*/

int
AllocValues_reclaim(ast_runtime* rt, AllocValues* allocvalues_v)
{
    int status = AST_NOERR;
    int i = 0;

    if(allocvalues_v->o_bytes_f.exists) {
        status = ast_reclaim(rt,ast_bytes,&allocvalues_v->o_bytes_f.value);
        if(!status) {goto done;}
    }
    for(i=0;i<allocvalues_v->r_string_f.count;i++) {
        status = ast_reclaim(rt,ast_string,&allocvalues_v->r_string_f.values[i]);
        if(!status) {goto done;}
    }
    ast_free(AllocValues->r_string.values)
    status = ast_reclaim(rt,ast_string,&allocvalues_v->a_string_f);
    if(!status) {goto done;}
    status = ast_reclaim(rt,ast_bytes,&allocvalues_v->a_bytes_f);
    if(!status) {goto done;}
    status = DefaultRequiredValues_reclaim(rt,allocvalues_v->a_mess_f);
    if(!status) {goto done;}

done:
    return status;

} /*AllocValues_reclaim*/

long
AllocValues_size(ast_runtime* rt, AllocValues* allocvalues_v)
{
    int status = AST_NOERR;
    int i = 0;
    long totalsize = 0;

    if(allocvalues_v->o_bytes_f.exists) {
        totalsize += ast_write_size(rt,ast_bytes,&allocvalues_v->o_bytes_f.value);
    }
    for(i=0;i<allocvalues_v->r_string_f.count;i++) {
        totalsize += ast_write_size(rt,ast_string,allocvalues_v->r_string_f.values[i]);
    }
    totalsize += ast_write_size(rt,ast_string,&allocvalues_v->a_string_f);
    totalsize += ast_write_size(rt,ast_bytes,&allocvalues_v->a_bytes_f);
    totalsize += DefaultRequiredValues_write_size(rt,5,allocvalues_v->a_mess_f);
    return status;

} /*AllocValues_write_size*/
