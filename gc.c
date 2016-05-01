#include "gc.h"

void _cs_gc_init(cs_gc_struct * g)
{
   long pages;
   void * cs_gc_memmap;

   /* check page size is 4kb */
   if (sysconf(_SC_PAGESIZE) != CS_PAGE_SIZE)
   {
      printf("Error: system page size does not equal 4kb!\n");
      exit(1);
   }

   /* get number of available pages of system memory */
   g->pages = sysconf(_SC_AVPHYS_PAGES)/2;

#if CS_GC_DEBUG
   printf("Number of available system pages = %ld\n", g->pages);
#endif

   do {
      g->memmap = mmap(NULL, CS_PAGE_SIZE*g->pages, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   } while (g->memmap == MAP_FAILED && (g->pages >>= 1) != 0);

#if CS_GC_DEBUG
   printf("Allocated %ld pages = %ld mb\n", g->pages, g->pages*CS_PAGE_SIZE/1024/1024);

   printf("Pointer = %p\n", g->memmap);
#endif
}

void _cs_gc_clear(cs_gc_struct * g)
{
   munmap(g->memmap, CS_PAGE_SIZE*g->pages);
}

