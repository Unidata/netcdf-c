#ifndef foo_H
#define foo_H



typedef enum TestEnumSmall {
    VALUE=0,
    OTHER_VALUE=1
} TestEnumSmall;


typedef enum TestEnum {
    VALUE0=0,
    VALUE1=1,
    VALUE127=127,
    VALUE128=128,
    VALUE16383=16383,
    VALUE16384=16384,
    VALUE2097151=2097151,
    VALUE2097152=2097152,
    VALUE268435455=268435455,
    VALUE268435456=268435456
} TestEnum;


typedef enum TestEnumDupValues {
    VALUE_A=42,
    VALUE_B=42,
    VALUE_C=42,
    VALUE_D=666,
    VALUE_E=666,
    VALUE_F=1000,
    VALUE_AA=1000,
    VALUE_BB=1001
} TestEnumDupValues;

typedef struct SubMess {
    int test_f;
} SubMess;

extern int SubMess_write(ast_runtime*,SubMess*);
extern int SubMess_read(ast_runtime*,SubMess**);
extern int SubMess_reclaim(ast_runtime*,SubMess*);
extern long SubMess_size(ast_runtime*,SubMess*);

typedef struct TestFieldNo15 {
    char* test_f;
} TestFieldNo15;

extern int TestFieldNo15_write(ast_runtime*,TestFieldNo15*);
extern int TestFieldNo15_read(ast_runtime*,TestFieldNo15**);
extern int TestFieldNo15_reclaim(ast_runtime*,TestFieldNo15*);
extern long TestFieldNo15_size(ast_runtime*,TestFieldNo15*);

typedef struct TestFieldNo16 {
    char* test_f;
} TestFieldNo16;

extern int TestFieldNo16_write(ast_runtime*,TestFieldNo16*);
extern int TestFieldNo16_read(ast_runtime*,TestFieldNo16**);
extern int TestFieldNo16_reclaim(ast_runtime*,TestFieldNo16*);
extern long TestFieldNo16_size(ast_runtime*,TestFieldNo16*);

typedef struct TestFieldNo2047 {
    char* test_f;
} TestFieldNo2047;

extern int TestFieldNo2047_write(ast_runtime*,TestFieldNo2047*);
extern int TestFieldNo2047_read(ast_runtime*,TestFieldNo2047**);
extern int TestFieldNo2047_reclaim(ast_runtime*,TestFieldNo2047*);
extern long TestFieldNo2047_size(ast_runtime*,TestFieldNo2047*);

typedef struct TestFieldNo2048 {
    char* test_f;
} TestFieldNo2048;

extern int TestFieldNo2048_write(ast_runtime*,TestFieldNo2048*);
extern int TestFieldNo2048_read(ast_runtime*,TestFieldNo2048**);
extern int TestFieldNo2048_reclaim(ast_runtime*,TestFieldNo2048*);
extern long TestFieldNo2048_size(ast_runtime*,TestFieldNo2048*);

typedef struct TestFieldNo262143 {
    char* test_f;
} TestFieldNo262143;

extern int TestFieldNo262143_write(ast_runtime*,TestFieldNo262143*);
extern int TestFieldNo262143_read(ast_runtime*,TestFieldNo262143**);
extern int TestFieldNo262143_reclaim(ast_runtime*,TestFieldNo262143*);
extern long TestFieldNo262143_size(ast_runtime*,TestFieldNo262143*);

typedef struct TestFieldNo262144 {
    char* test_f;
} TestFieldNo262144;

extern int TestFieldNo262144_write(ast_runtime*,TestFieldNo262144*);
extern int TestFieldNo262144_read(ast_runtime*,TestFieldNo262144**);
extern int TestFieldNo262144_reclaim(ast_runtime*,TestFieldNo262144*);
extern long TestFieldNo262144_size(ast_runtime*,TestFieldNo262144*);

typedef struct TestFieldNo33554431 {
    char* test_f;
} TestFieldNo33554431;

extern int TestFieldNo33554431_write(ast_runtime*,TestFieldNo33554431*);
extern int TestFieldNo33554431_read(ast_runtime*,TestFieldNo33554431**);
extern int TestFieldNo33554431_reclaim(ast_runtime*,TestFieldNo33554431*);
extern long TestFieldNo33554431_size(ast_runtime*,TestFieldNo33554431*);

typedef struct TestFieldNo33554432 {
    char* test_f;
} TestFieldNo33554432;

extern int TestFieldNo33554432_write(ast_runtime*,TestFieldNo33554432*);
extern int TestFieldNo33554432_read(ast_runtime*,TestFieldNo33554432**);
extern int TestFieldNo33554432_reclaim(ast_runtime*,TestFieldNo33554432*);
extern long TestFieldNo33554432_size(ast_runtime*,TestFieldNo33554432*);

typedef struct TestMess {
    struct {int count; int* values;} test_int32_f;
    struct {int count; int* values;} test_sint32_f;
    struct {int count; int* values;} test_sfixed32_f;
    struct {int count; longlong* values;} test_int64_f;
    struct {int count; longlong* values;} test_sint64_f;
    struct {int count; longlong* values;} test_sfixed64_f;
    struct {int count; int* values;} test_uint32_f;
    struct {int count; int* values;} test_fixed32_f;
    struct {int count; longlong* values;} test_uint64_f;
    struct {int count; longlong* values;} test_fixed64_f;
    struct {int count; float* values;} test_float_f;
    struct {int count; double* values;} test_double_f;
    struct {int count; int* values;} test_boolean_f;
    struct {int count; TestEnumSmall** values;} test_enum_small_f;
    struct {int count; TestEnum** values;} test_enum_f;
    struct {int count; char** values;} test_string_f;
    struct {int count; Bytestring* values;} test_bytes_f;
    struct {int count; SubMess** values;} test_message_f;
} TestMess;

extern int TestMess_write(ast_runtime*,TestMess*);
extern int TestMess_read(ast_runtime*,TestMess**);
extern int TestMess_reclaim(ast_runtime*,TestMess*);
extern long TestMess_size(ast_runtime*,TestMess*);

typedef struct TestMessPacked {
    struct {int count; int* values;} test_int32_f;
    struct {int count; int* values;} test_sint32_f;
    struct {int count; int* values;} test_sfixed32_f;
    struct {int count; longlong* values;} test_int64_f;
    struct {int count; longlong* values;} test_sint64_f;
    struct {int count; longlong* values;} test_sfixed64_f;
    struct {int count; int* values;} test_uint32_f;
    struct {int count; int* values;} test_fixed32_f;
    struct {int count; longlong* values;} test_uint64_f;
    struct {int count; longlong* values;} test_fixed64_f;
    struct {int count; float* values;} test_float_f;
    struct {int count; double* values;} test_double_f;
    struct {int count; int* values;} test_boolean_f;
    struct {int count; TestEnumSmall** values;} test_enum_small_f;
    struct {int count; TestEnum** values;} test_enum_f;
} TestMessPacked;

extern int TestMessPacked_write(ast_runtime*,TestMessPacked*);
extern int TestMessPacked_read(ast_runtime*,TestMessPacked**);
extern int TestMessPacked_reclaim(ast_runtime*,TestMessPacked*);
extern long TestMessPacked_size(ast_runtime*,TestMessPacked*);

typedef struct TestMessOptional {
    struct {int exists; int value;} test_int32_f;
    struct {int exists; int value;} test_sint32_f;
    struct {int exists; int value;} test_sfixed32_f;
    struct {int exists; longlong value;} test_int64_f;
    struct {int exists; longlong value;} test_sint64_f;
    struct {int exists; longlong value;} test_sfixed64_f;
    struct {int exists; int value;} test_uint32_f;
    struct {int exists; int value;} test_fixed32_f;
    struct {int exists; longlong value;} test_uint64_f;
    struct {int exists; longlong value;} test_fixed64_f;
    struct {int exists; float value;} test_float_f;
    struct {int exists; double value;} test_double_f;
    struct {int exists; int value;} test_boolean_f;
    struct {int exists; TestEnumSmall* value;} test_enum_small_f;
    struct {int exists; TestEnum* value;} test_enum_f;
    struct {int exists; char* value;} test_string_f;
    struct {int exists; Bytestring value;} test_bytes_f;
    struct {int exists; SubMess* value;} test_message_f;
} TestMessOptional;

extern int TestMessOptional_write(ast_runtime*,TestMessOptional*);
extern int TestMessOptional_read(ast_runtime*,TestMessOptional**);
extern int TestMessOptional_reclaim(ast_runtime*,TestMessOptional*);
extern long TestMessOptional_size(ast_runtime*,TestMessOptional*);

typedef struct TestMessRequiredInt32 {
    int test_f;
} TestMessRequiredInt32;

extern int TestMessRequiredInt32_write(ast_runtime*,TestMessRequiredInt32*);
extern int TestMessRequiredInt32_read(ast_runtime*,TestMessRequiredInt32**);
extern int TestMessRequiredInt32_reclaim(ast_runtime*,TestMessRequiredInt32*);
extern long TestMessRequiredInt32_size(ast_runtime*,TestMessRequiredInt32*);

typedef struct TestMessRequiredSInt32 {
    int test_f;
} TestMessRequiredSInt32;

extern int TestMessRequiredSInt32_write(ast_runtime*,TestMessRequiredSInt32*);
extern int TestMessRequiredSInt32_read(ast_runtime*,TestMessRequiredSInt32**);
extern int TestMessRequiredSInt32_reclaim(ast_runtime*,TestMessRequiredSInt32*);
extern long TestMessRequiredSInt32_size(ast_runtime*,TestMessRequiredSInt32*);

typedef struct TestMessRequiredSFixed32 {
    int test_f;
} TestMessRequiredSFixed32;

extern int TestMessRequiredSFixed32_write(ast_runtime*,TestMessRequiredSFixed32*);
extern int TestMessRequiredSFixed32_read(ast_runtime*,TestMessRequiredSFixed32**);
extern int TestMessRequiredSFixed32_reclaim(ast_runtime*,TestMessRequiredSFixed32*);
extern long TestMessRequiredSFixed32_size(ast_runtime*,TestMessRequiredSFixed32*);

typedef struct TestMessRequiredInt64 {
    longlong test_f;
} TestMessRequiredInt64;

extern int TestMessRequiredInt64_write(ast_runtime*,TestMessRequiredInt64*);
extern int TestMessRequiredInt64_read(ast_runtime*,TestMessRequiredInt64**);
extern int TestMessRequiredInt64_reclaim(ast_runtime*,TestMessRequiredInt64*);
extern long TestMessRequiredInt64_size(ast_runtime*,TestMessRequiredInt64*);

typedef struct TestMessRequiredSInt64 {
    longlong test_f;
} TestMessRequiredSInt64;

extern int TestMessRequiredSInt64_write(ast_runtime*,TestMessRequiredSInt64*);
extern int TestMessRequiredSInt64_read(ast_runtime*,TestMessRequiredSInt64**);
extern int TestMessRequiredSInt64_reclaim(ast_runtime*,TestMessRequiredSInt64*);
extern long TestMessRequiredSInt64_size(ast_runtime*,TestMessRequiredSInt64*);

typedef struct TestMessRequiredSFixed64 {
    longlong test_f;
} TestMessRequiredSFixed64;

extern int TestMessRequiredSFixed64_write(ast_runtime*,TestMessRequiredSFixed64*);
extern int TestMessRequiredSFixed64_read(ast_runtime*,TestMessRequiredSFixed64**);
extern int TestMessRequiredSFixed64_reclaim(ast_runtime*,TestMessRequiredSFixed64*);
extern long TestMessRequiredSFixed64_size(ast_runtime*,TestMessRequiredSFixed64*);

typedef struct TestMessRequiredUInt32 {
    int test_f;
} TestMessRequiredUInt32;

extern int TestMessRequiredUInt32_write(ast_runtime*,TestMessRequiredUInt32*);
extern int TestMessRequiredUInt32_read(ast_runtime*,TestMessRequiredUInt32**);
extern int TestMessRequiredUInt32_reclaim(ast_runtime*,TestMessRequiredUInt32*);
extern long TestMessRequiredUInt32_size(ast_runtime*,TestMessRequiredUInt32*);

typedef struct TestMessRequiredFixed32 {
    int test_f;
} TestMessRequiredFixed32;

extern int TestMessRequiredFixed32_write(ast_runtime*,TestMessRequiredFixed32*);
extern int TestMessRequiredFixed32_read(ast_runtime*,TestMessRequiredFixed32**);
extern int TestMessRequiredFixed32_reclaim(ast_runtime*,TestMessRequiredFixed32*);
extern long TestMessRequiredFixed32_size(ast_runtime*,TestMessRequiredFixed32*);

typedef struct TestMessRequiredUInt64 {
    longlong test_f;
} TestMessRequiredUInt64;

extern int TestMessRequiredUInt64_write(ast_runtime*,TestMessRequiredUInt64*);
extern int TestMessRequiredUInt64_read(ast_runtime*,TestMessRequiredUInt64**);
extern int TestMessRequiredUInt64_reclaim(ast_runtime*,TestMessRequiredUInt64*);
extern long TestMessRequiredUInt64_size(ast_runtime*,TestMessRequiredUInt64*);

typedef struct TestMessRequiredFixed64 {
    longlong test_f;
} TestMessRequiredFixed64;

extern int TestMessRequiredFixed64_write(ast_runtime*,TestMessRequiredFixed64*);
extern int TestMessRequiredFixed64_read(ast_runtime*,TestMessRequiredFixed64**);
extern int TestMessRequiredFixed64_reclaim(ast_runtime*,TestMessRequiredFixed64*);
extern long TestMessRequiredFixed64_size(ast_runtime*,TestMessRequiredFixed64*);

typedef struct TestMessRequiredFloat {
    float test_f;
} TestMessRequiredFloat;

extern int TestMessRequiredFloat_write(ast_runtime*,TestMessRequiredFloat*);
extern int TestMessRequiredFloat_read(ast_runtime*,TestMessRequiredFloat**);
extern int TestMessRequiredFloat_reclaim(ast_runtime*,TestMessRequiredFloat*);
extern long TestMessRequiredFloat_size(ast_runtime*,TestMessRequiredFloat*);

typedef struct TestMessRequiredDouble {
    double test_f;
} TestMessRequiredDouble;

extern int TestMessRequiredDouble_write(ast_runtime*,TestMessRequiredDouble*);
extern int TestMessRequiredDouble_read(ast_runtime*,TestMessRequiredDouble**);
extern int TestMessRequiredDouble_reclaim(ast_runtime*,TestMessRequiredDouble*);
extern long TestMessRequiredDouble_size(ast_runtime*,TestMessRequiredDouble*);

typedef struct TestMessRequiredBool {
    int test_f;
} TestMessRequiredBool;

extern int TestMessRequiredBool_write(ast_runtime*,TestMessRequiredBool*);
extern int TestMessRequiredBool_read(ast_runtime*,TestMessRequiredBool**);
extern int TestMessRequiredBool_reclaim(ast_runtime*,TestMessRequiredBool*);
extern long TestMessRequiredBool_size(ast_runtime*,TestMessRequiredBool*);

typedef struct TestMessRequiredEnum {
    TestEnum* test_f;
} TestMessRequiredEnum;

extern int TestMessRequiredEnum_write(ast_runtime*,TestMessRequiredEnum*);
extern int TestMessRequiredEnum_read(ast_runtime*,TestMessRequiredEnum**);
extern int TestMessRequiredEnum_reclaim(ast_runtime*,TestMessRequiredEnum*);
extern long TestMessRequiredEnum_size(ast_runtime*,TestMessRequiredEnum*);

typedef struct TestMessRequiredEnumSmall {
    TestEnumSmall* test_f;
} TestMessRequiredEnumSmall;

extern int TestMessRequiredEnumSmall_write(ast_runtime*,TestMessRequiredEnumSmall*);
extern int TestMessRequiredEnumSmall_read(ast_runtime*,TestMessRequiredEnumSmall**);
extern int TestMessRequiredEnumSmall_reclaim(ast_runtime*,TestMessRequiredEnumSmall*);
extern long TestMessRequiredEnumSmall_size(ast_runtime*,TestMessRequiredEnumSmall*);

typedef struct TestMessRequiredString {
    char* test_f;
} TestMessRequiredString;

extern int TestMessRequiredString_write(ast_runtime*,TestMessRequiredString*);
extern int TestMessRequiredString_read(ast_runtime*,TestMessRequiredString**);
extern int TestMessRequiredString_reclaim(ast_runtime*,TestMessRequiredString*);
extern long TestMessRequiredString_size(ast_runtime*,TestMessRequiredString*);

typedef struct TestMessRequiredBytes {
    Bytestring test_f;
} TestMessRequiredBytes;

extern int TestMessRequiredBytes_write(ast_runtime*,TestMessRequiredBytes*);
extern int TestMessRequiredBytes_read(ast_runtime*,TestMessRequiredBytes**);
extern int TestMessRequiredBytes_reclaim(ast_runtime*,TestMessRequiredBytes*);
extern long TestMessRequiredBytes_size(ast_runtime*,TestMessRequiredBytes*);

typedef struct TestMessRequiredMessage {
    SubMess* test_f;
} TestMessRequiredMessage;

extern int TestMessRequiredMessage_write(ast_runtime*,TestMessRequiredMessage*);
extern int TestMessRequiredMessage_read(ast_runtime*,TestMessRequiredMessage**);
extern int TestMessRequiredMessage_reclaim(ast_runtime*,TestMessRequiredMessage*);
extern long TestMessRequiredMessage_size(ast_runtime*,TestMessRequiredMessage*);

typedef struct EmptyMess {
} EmptyMess;

extern int EmptyMess_write(ast_runtime*,EmptyMess*);
extern int EmptyMess_read(ast_runtime*,EmptyMess**);
extern int EmptyMess_reclaim(ast_runtime*,EmptyMess*);
extern long EmptyMess_size(ast_runtime*,EmptyMess*);

typedef struct DefaultRequiredValues {
    int v_int32_f;
    int v_uint32_f;
    int v_int64_f;
    int v_uint64_f;
    float v_float_f;
    double v_double_f;
    char* v_string_f;
    Bytestring v_bytes_f;
} DefaultRequiredValues;

extern int DefaultRequiredValues_write(ast_runtime*,DefaultRequiredValues*);
extern int DefaultRequiredValues_read(ast_runtime*,DefaultRequiredValues**);
extern int DefaultRequiredValues_reclaim(ast_runtime*,DefaultRequiredValues*);
extern long DefaultRequiredValues_size(ast_runtime*,DefaultRequiredValues*);

typedef struct DefaultOptionalValues {
    struct {int exists; int value;} v_int32_f;
    struct {int exists; int value;} v_uint32_f;
    struct {int exists; int value;} v_int64_f;
    struct {int exists; int value;} v_uint64_f;
    struct {int exists; float value;} v_float_f;
    struct {int exists; double value;} v_double_f;
    struct {int exists; char* value;} v_string_f;
    struct {int exists; Bytestring value;} v_bytes_f;
} DefaultOptionalValues;

extern int DefaultOptionalValues_write(ast_runtime*,DefaultOptionalValues*);
extern int DefaultOptionalValues_read(ast_runtime*,DefaultOptionalValues**);
extern int DefaultOptionalValues_reclaim(ast_runtime*,DefaultOptionalValues*);
extern long DefaultOptionalValues_size(ast_runtime*,DefaultOptionalValues*);

typedef struct AllocValues {
    struct {int exists; Bytestring value;} o_bytes_f;
    struct {int count; char** values;} r_string_f;
    char* a_string_f;
    Bytestring a_bytes_f;
    DefaultRequiredValues* a_mess_f;
} AllocValues;

extern int AllocValues_write(ast_runtime*,AllocValues*);
extern int AllocValues_read(ast_runtime*,AllocValues**);
extern int AllocValues_reclaim(ast_runtime*,AllocValues*);
extern long AllocValues_size(ast_runtime*,AllocValues*);

#endif /*foo_H*/
