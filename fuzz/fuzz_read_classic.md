# Classic reader fuzzing

`fuzz_read_classic.c` opens CDF-1, CDF-2, and CDF-5 input through
`nc_open_mem`, then exercises bounded attribute reads, variable reads,
strided reads, and numeric conversions.

The harness rejects inputs shorter than 32 bytes to avoid the classic
reader's initial-window assertion with the memory backend. Before opening
the input, an allocation-free header preflight checks that names, dimension
IDs, and attribute payloads fit in the supplied bytes. It also limits the
combined number of dimensions, variables, and attributes to 4,096. These
checks prevent oversized metadata declarations from driving allocations
before the harness's post-open read limits can take effect.

Variable types must be in the range NC_BYTE through NC_UINT64 (1 through 11),
matching the types handled by ncx_szof without asserting.

Record counts, dimension lengths, declared variable sizes,
and data offsets are left for netCDF to interpret. The preflight is a fuzzing
resource policy, not a replacement for library-side validation or a guarantee
against all excessive resource use. It deliberately sacrifices coverage of
truncated headers, unsupported attribute and variable types, nonempty lists with incorrect
tags, and metadata exceeding the entry budget. It does not repair the
underlying library's unchecked allocations.