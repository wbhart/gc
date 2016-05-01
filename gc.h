#include <stdlib.h>   /* memmap */
#include <unistd.h>   /* sysconf */
#include <stdio.h>    /* printf */
#include <sys/mman.h> /* mmap, munmap */
#include <errno.h>

#ifndef GC_H
#define GC_H

#ifdef __cplusplus
 extern "C" {
#endif

#define CS_GC_DEBUG  1    /* print debug information */
#define CS_PAGE_SIZE 4096 /* system page size */

typedef struct
{
   long pages;
   void * memmap;
} cs_gc_struct;

void _cs_gc_init(cs_gc_struct * g);

void _cs_gc_clear(cs_gc_struct * g);

#ifdef __cplusplus
 }
#endif

#endif

