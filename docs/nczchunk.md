NetCDF ZARR Chunking
====================================

# Abstract

*This document describes the internal algorithms associated
with chunk support. This is based on the Python implementation
because I cannot find any complete C++ or Java implementation.

*Distribution of this document is currently restricted to Unidata.*

[TOC]

# Introduction {#nczchunk_intro}

This document provides an overview of how NCZarr implements
the chunking-related algorithms. Unfortunately, we could
not find any kind of detailed description on the Web of
chunking algorithms. So, these algorithms are taken directly
from the Python Zarr implementation.

# Chunking Overview {#nczchunk_overview}

Assume we have a sequence of dimension lengths (D0,...Dn)
and for each Di, an associated chunk length Ci.
A point in the dimension space is a sequence of values
(d0,...dn) where for each di, 0 <= di < Di.
Each chunk is assumed to be identified by the least point
contained in the chunk.
<!--We also assume that each such point can be converted into a
linear index in the space ````0...(D0 * D1...*Dn)```` and vice versa.-->

The basic operations needed for chunking are as follows.
* Compute Chunk Remainders: the length of the short chunk
  with respect to the dimension length.
* Compute the total number of chunks.
* Compute chunk sizes: given (D0...Dn) and (C0...Cn), compute
  the chunk sizes, including the possibly truncated chunk sizes
  if the dimension modulo chunk size is not zero.
* Chunk Identifiers: assign an n-dimensional index defining the chunk;
  effectively the least point in the each chunk.
* Point intersection: given a point in the dimension space
  (d0...dn), find the identifier of the chunk containing that point.
* Hyperslab intersection: given a hyperslab identified by a
  start point and an edge length, compute the set of chunks
  that cover that hyperslab.  Note that stride is not used
  because it can be folded into the edge length.
   
# Compute Short Chunk Lengths
The short chunk lengths are (Sh0,...Shn)
where ````Shi = (Ci%Di == 0 ? Ci : Ci/Di)````,

# Compute Chunk Count

The total number of chunks is going to be
````N = (X0 * X1 * ... Xn)````,
where ````Xi = ((Di+Ci-1) / Ci)````.

# Compute Chunk Sizes

So, the chunk sizes are 

````((C0/D0) * (C1/D1 * ... (Cn/Dn))````.


# Chunk Identifiers

for i=0;i<nchunks;i++ {
    leastpoint[i] = (p0,p1,...pn)
    where pi = i*Ci
}

# Point Intersection

Given a point (p0,...pn)
containing_chunk is C with least point (c0,...cn)
is the chunk such that
for(i=0;i<n;i++) {
    ci <= pi < ci + Ci
}

# Hyperslab Intersection

Given a strided hyperslab H with
start = (s0,...sn)
edge = (e0,...en)
stride = (st9,...stn)

Compute bounding box for hyperslab where
bbstart = start
for i=0;i<n;i++)
    bbedge[i] = edge[i]*stride[i]


# Notes
1. '/' is always integer division
