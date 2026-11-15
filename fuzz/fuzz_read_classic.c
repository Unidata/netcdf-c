#include <stdint.h>
#include <stddef.h>
#include <netcdf.h>
#include <netcdf_mem.h>

#define MAX_INPUT_SIZE (1024 * 1024)
#define MAX_VARIABLES 16
#define MAX_ATTRIBUTES 8
#define MAX_RANK 8
#define MAX_ELEMENTS 256
#define MAX_HEADER_ENTRIES 4096

struct header_cursor {
    const uint8_t *data;
    size_t remaining;
    size_t width;
    size_t entries_left;
};

static int header_skip(struct header_cursor *cursor, size_t length)
{
    if (length > cursor->remaining) {
        return 0;
    }
    cursor->data += length;
    cursor->remaining -= length;
    return 1;
}

static int header_integer(struct header_cursor *cursor, size_t width, uint64_t *value)
{
    if (width > cursor->remaining) {
        return 0;
    }
    *value = 0;
    for (size_t byte = 0; byte < width; byte++) {
        *value = (*value << 8) | cursor->data[byte];
    }
    return header_skip(cursor, width);
}

static int header_name(struct header_cursor *cursor)
{
    uint64_t length;
    if (!header_integer(cursor, cursor->width, &length) || length > cursor->remaining) {
        return 0;
    }
    size_t padding = (4 - (size_t)length % 4) % 4;
    return header_skip(cursor, (size_t)length) && header_skip(cursor, padding);
}

static int header_list(struct header_cursor *cursor, uint64_t expected, uint64_t *count)
{
    uint64_t tag;
    if (!header_integer(cursor, 4, &tag) ||
        !header_integer(cursor, cursor->width, count) ||
        (*count != 0 && tag != expected) ||
        *count > cursor->entries_left || *count > cursor->remaining / cursor->width) {
        return 0;
    }
    cursor->entries_left -= (size_t)*count;
    return 1;
}

static int header_attributes(struct header_cursor *cursor)
{
    static const size_t type_sizes[] = {0, 1, 1, 2, 4, 4, 8, 1, 2, 4, 8, 8};
    uint64_t count;
    if (!header_list(cursor, 12, &count)) {
        return 0;
    }
    for (uint64_t attribute = 0; attribute < count; attribute++) {
        uint64_t type, length;
        if (!header_name(cursor) || !header_integer(cursor, 4, &type) ||
            type == 0 || type >= sizeof(type_sizes) / sizeof(type_sizes[0]) ||
            !header_integer(cursor, cursor->width, &length) ||
            length > cursor->remaining / type_sizes[type]) {
            return 0;
        }
        size_t bytes = (size_t)length * type_sizes[type];
        size_t padding = (4 - bytes % 4) % 4;
        if (!header_skip(cursor, bytes) || !header_skip(cursor, padding)) {
            return 0;
        }
    }
    return 1;
}

static int header_within_budget(const uint8_t *data, size_t size)
{
    struct header_cursor cursor = {data + 4, size - 4, data[3] == 5 ? 8 : 4,
                                   MAX_HEADER_ENTRIES};
    uint64_t count;
    if (!header_skip(&cursor, cursor.width) || !header_list(&cursor, 10, &count)) {
        return 0;
    }
    for (uint64_t dimension = 0; dimension < count; dimension++) {
        if (!header_name(&cursor) || !header_skip(&cursor, cursor.width)) {
            return 0;
        }
    }
    if (!header_attributes(&cursor) || !header_list(&cursor, 11, &count)) {
        return 0;
    }
    for (uint64_t variable = 0; variable < count; variable++) {
        uint64_t rank, type;
        if (!header_name(&cursor) || !header_integer(&cursor, cursor.width, &rank) ||
            rank > cursor.remaining / cursor.width ||
            !header_skip(&cursor, (size_t)rank * cursor.width) ||
            !header_attributes(&cursor) ||
            !header_integer(&cursor, 4, &type) || type < NC_BYTE || type > NC_UINT64 ||
            !header_skip(&cursor, cursor.width + (data[3] == 1 ? 4 : 8))) {
            return 0;
        }
    }
    return 1;
}

static void read_attributes(int ncid, int varid, int natts)
{
    for (int attribute = 0; attribute < natts && attribute < MAX_ATTRIBUTES; attribute++) {
        char name[NC_MAX_NAME + 1];
        nc_type type;
        size_t length;

        if (nc_inq_attname(ncid, varid, attribute, name) != NC_NOERR ||
            nc_inq_att(ncid, varid, name, &type, &length) != NC_NOERR ||
            length > MAX_ELEMENTS) {
            continue;
        }

        if (type == NC_CHAR) {
            char values[MAX_ELEMENTS];
            (void)nc_get_att_text(ncid, varid, name, values);
        } else {
            double values[MAX_ELEMENTS];
            int integers[MAX_ELEMENTS];
            (void)nc_get_att_double(ncid, varid, name, values);
            (void)nc_get_att_int(ncid, varid, name, integers);
        }
    }
}

static void read_variable(int ncid, int varid, uint8_t selector)
{
    int rank, natts;
    int dimensions[MAX_RANK];
    nc_type type;
    size_t start[MAX_RANK] = {0};
    size_t count[MAX_RANK] = {0};
    ptrdiff_t stride[MAX_RANK] = {0};
    size_t budget = MAX_ELEMENTS;

    if (nc_inq_varndims(ncid, varid, &rank) != NC_NOERR ||
        rank < 0 || rank > MAX_RANK ||
        nc_inq_var(ncid, varid, NULL, &type, NULL, dimensions, &natts) != NC_NOERR) {
        return;
    }

    read_attributes(ncid, varid, natts);

    for (int dimension = 0; dimension < rank; dimension++) {
        size_t length;
        if (nc_inq_dimlen(ncid, dimensions[dimension], &length) != NC_NOERR || length == 0) {
            return;
        }

        start[dimension] = (selector & 1) ? length - 1 : 0;
        stride[dimension] = 1 + ((selector >> 1) & 3);
        size_t available = 1 + (length - 1 - start[dimension]) / (size_t)stride[dimension];
        count[dimension] = available < budget ? available : budget;
        budget /= count[dimension];
    }

    if (type == NC_CHAR) {
        char values[MAX_ELEMENTS];
        (void)nc_get_vara_text(ncid, varid, start, count, values);
        (void)nc_get_vars_text(ncid, varid, start, count, stride, values);
    } else {
        double values[MAX_ELEMENTS];
        (void)nc_get_vara_double(ncid, varid, start, count, values);
        (void)nc_get_vars_double(ncid, varid, start, count, stride, values);

        switch ((selector >> 3) % 3) {
        case 0: {
            int integers[MAX_ELEMENTS];
            (void)nc_get_vara_int(ncid, varid, start, count, integers);
            (void)nc_get_vars_int(ncid, varid, start, count, stride, integers);
            break;
        }
        case 1: {
            long long integers[MAX_ELEMENTS];
            (void)nc_get_vara_longlong(ncid, varid, start, count, integers);
            (void)nc_get_vars_longlong(ncid, varid, start, count, stride, integers);
            break;
        }
        default: {
            unsigned long long integers[MAX_ELEMENTS];
            (void)nc_get_vara_ulonglong(ncid, varid, start, count, integers);
            (void)nc_get_vars_ulonglong(ncid, varid, start, count, stride, integers);
            break;
        }
        }
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    int ncid, nvars, natts;

    if (size < 32 || size > MAX_INPUT_SIZE ||
        data[0] != 'C' || data[1] != 'D' || data[2] != 'F' ||
        (data[3] != 1 && data[3] != 2 && data[3] != 5)) {
        return 0;
    }

    if (!header_within_budget(data, size) ||
        nc_open_mem("fuzz.nc", NC_NOWRITE, size, (void *)data, &ncid) != NC_NOERR) {
        return 0;
    }

    if (nc_inq(ncid, NULL, &nvars, &natts, NULL) == NC_NOERR) {
        read_attributes(ncid, NC_GLOBAL, natts);
        for (int variable = 0; variable < nvars && variable < MAX_VARIABLES; variable++) {
            uint8_t selector = data[size - 1] ^ (uint8_t)variable;
            read_variable(ncid, variable, selector);
        }
    }

    (void)nc_close(ncid);
    return 0;
}