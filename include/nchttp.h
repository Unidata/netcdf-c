/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * Header file for dhttp.c
 * @author Dennis Heimbigner
 */

#ifndef NCHTTP_H
#define NCHTTP_H

extern int nc_http_open(const char* objecturl, void** curlp, size64_t* filelenp);
extern int nc_http_read(void* curl, const char* url, size64_t start, size64_t count, NCbytes* buf);
extern int nc_http_close(void* curl);

#endif /*NCHTTP_H*/
