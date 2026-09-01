/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *	See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

/*
 * The slice of <sys/mman.h> that libsrc/mmapio.c actually uses, implemented on
 * the Win32 section API. mmapio.c needs exactly two anonymous-vs-file cases and
 * no mprotect, no msync (MAP_SHARED writes are coherent with the file on
 * Windows as they are on POSIX) and no partial unmapping, so the shim is small.
 *
 * Both cases go through CreateFileMapping + MapViewOfFile rather than
 * VirtualAlloc for the anonymous one, so that a single munmap() --
 * UnmapViewOfFile -- releases either kind without having to remember which it
 * was. The section handle is closed as soon as the view exists: Windows keeps
 * the section alive for as long as a view of it is mapped.
 *
 * Offsets are always zero here. If that ever changes, note that MapViewOfFile
 * requires the offset to be a multiple of the allocation granularity (64 KB),
 * not of the page size.
 */

#ifndef NCWINMMAP_H
#define NCWINMMAP_H

#ifdef _WIN32

#include <windows.h>
#include <io.h>
#include <errno.h>
#include <stdlib.h>

#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2

#define MAP_FILE      0x00
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS

#define MAP_FAILED ((void*)-1)

static void*
nc_win_mmap(void* addr, size_t len, int prot, int flags, int fd, long long offset)
{
    HANDLE section = NULL;
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD protect;
    DWORD access;
    ULARGE_INTEGER maxsize;
    ULARGE_INTEGER off;
    void* view = NULL;

    (void)addr; /* a fixed mapping address is never asked for here */

    if(len == 0) {errno = EINVAL; return MAP_FAILED;}

    if(prot & PROT_WRITE) {
	protect = PAGE_READWRITE;
	access = (flags & MAP_PRIVATE) ? FILE_MAP_COPY : FILE_MAP_WRITE;
    } else if(prot & PROT_READ) {
	protect = PAGE_READONLY;
	access = FILE_MAP_READ;
    } else {
	errno = EINVAL;
	return MAP_FAILED;
    }

    if(!(flags & MAP_ANONYMOUS)) {
	file = (HANDLE)_get_osfhandle(fd);
	if(file == INVALID_HANDLE_VALUE) {errno = EBADF; return MAP_FAILED;}
	/* A file-backed section is sized by the file when maxsize is zero, and
	   mmapio.c has already grown the file to at least len. */
	maxsize.QuadPart = 0;
    } else {
	/* Pagefile-backed: the size has to be stated. FILE_MAP_COPY on such a
	   section would make the pages private but leave nothing to be private
	   from, so an anonymous mapping is always mapped for write. */
	file = INVALID_HANDLE_VALUE;
	maxsize.QuadPart = (ULONGLONG)offset + (ULONGLONG)len;
	access = FILE_MAP_WRITE;
	protect = PAGE_READWRITE;
    }

    section = CreateFileMappingA(file, NULL, protect,
				 maxsize.HighPart, maxsize.LowPart, NULL);
    if(section == NULL) {errno = ENOMEM; return MAP_FAILED;}

    off.QuadPart = (ULONGLONG)offset;
    view = MapViewOfFile(section, access, off.HighPart, off.LowPart, len);

    /* The view holds its own reference to the section. */
    CloseHandle(section);

    if(view == NULL) {errno = ENOMEM; return MAP_FAILED;}
    return view;
}

static int
nc_win_munmap(void* addr, size_t len)
{
    (void)len; /* Windows unmaps whole views only, which is all mmapio.c does */
    if(addr == NULL || addr == MAP_FAILED) return 0;
    if(!UnmapViewOfFile(addr)) {errno = EINVAL; return -1;}
    return 0;
}

#define mmap(a,l,p,f,fd,o)  nc_win_mmap((a),(l),(p),(f),(fd),(long long)(o))
#define munmap(a,l)         nc_win_munmap((a),(l))

#endif /*_WIN32*/

#endif /*NCWINMMAP_H*/
