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
#define CS_WORD_BITS 64   /* bits per word */

typedef struct cs_gc_virtual_struct
{
   struct cs_gc_virtual_struct * next;
   unsigned long * active;
   unsigned long * reachable;
   void * start;
   void * end;
   long pages;
} cs_gc_virtual_struct;

typedef struct cs_gc_page_struct
{
   struct cs_gc_page_struct * next;
} cs_gc_page_struct;

typedef struct
{
   cs_gc_virtual_struct * virtual_list;
   long pages;
   void * memmap;
   void * memmap_ptr;
   cs_gc_page_struct * page_list[CS_WORD_BITS + 1];
} cs_gc_struct;

void _cs_gc_allocate_virtual(cs_gc_struct * g, int follow_on);

void _cs_gc_init(cs_gc_struct * g);

void _cs_gc_clear(cs_gc_struct * g);

void * _cs_gc_get_pages(cs_gc_struct * g, long n);

void _cs_gc_free_pages(cs_gc_struct * g, void * ptr, long n);

#ifdef __cplusplus
 }
#endif

#endif

