#include "gc.h"

/*******************************************************************************
*
*  Allocating large blocks of virtual memory from the OS
*
*******************************************************************************/

void _cs_gc_allocate_virtual(cs_gc_struct * g, int follow_on)
{
   /* new allocation follows the last, or can be anywhere */
   void * ptr = follow_on ? g->memmap_ptr + g->pages*CS_PAGE_SIZE : NULL;
   long i, words, pages;
   cs_gc_virtual_struct * vb;

   /* get number of available pages of system memory */
   g->pages = sysconf(_SC_AVPHYS_PAGES);

#if CS_GC_DEBUG
   printf("Number of available system pages = %ld\n", g->pages);
#endif

   do {
      g->pages >>= 1;
      g->memmap = mmap(ptr, CS_PAGE_SIZE*g->pages, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   } while (g->memmap == MAP_FAILED);

   if (g->pages < 4)
   {
      printf("Error: unable to allocate pages\n");
      exit(1);
   }

#if CS_GC_DEBUG
   printf("Allocated %ld pages = %ld mb\n", g->pages,
                                            g->pages*CS_PAGE_SIZE/1024/1024);

   printf("Pointer = %p\n", g->memmap);
#endif

   /* how many pages to be reserved for active/reachable tables */

   pages = (g->pages + 8*CS_PAGE_SIZE - 1)/(8*CS_PAGE_SIZE);
   pages = pages*2 + 1;

   /* set up header */

   vb = (cs_gc_virtual_struct *) g->memmap;
   
   vb->next = follow_on ? g->virtual_list : NULL;
   g->virtual_list = vb;

   vb->active = g->memmap + CS_PAGE_SIZE;
   vb->reachable = g->memmap + (pages + 1)*CS_PAGE_SIZE;
   vb->start = g->memmap + (2*pages + 1)*CS_PAGE_SIZE;

   /* skip 2*pages + 1 pages for active and reachable tables and header */

   pages = pages*2 + 1;
   g->memmap_ptr = g->memmap + pages*CS_PAGE_SIZE; /* next available page */
   g->pages -= pages;

   vb->pages = g->pages;
   vb->end = vb->start + vb->pages*CS_PAGE_SIZE;

   /* set up active bitlist */

   words = (g->pages + CS_WORD_BITS - 1)/CS_WORD_BITS;

   for (i = 0; i < words; i++)
      vb->active[i] = 0UL;
}

void _cs_gc_init(cs_gc_struct * g)
{
   long pages, i;
   void * cs_gc_memmap;

   /* check page size is 4kb */
   if (sysconf(_SC_PAGESIZE) != CS_PAGE_SIZE)
   {
      printf("Error: system page size does not equal 4kb!\n");
      exit(1);
   }
   
   /* allcate as much virtual memory as we dare */
   _cs_gc_allocate_virtual(g, 0);

   /* initialise pools for object sizes from 0 to 64 words */
   for (i = 0; i <= 64; i++)
   {
      g->page_list[i] = (cs_gc_page_struct *) _cs_gc_get_pages(g, 1);
   }
}

void _cs_gc_clear(cs_gc_struct * g)
{
   long i;

   /* free unused pages */
   _cs_gc_free_pages(g, g->memmap_ptr, g->pages);

   /* clean up pools */
   for (i = 0; i <= 64; i++)
      _cs_gc_free_pages(g, g->page_list[i], 1);

#if CS_GC_DEBUG
   if (g->virtual_list != NULL)
   {
      printf("Warning: cleanup failed\n");
      
      while (g->virtual_list != NULL)
      {
         printf("Orphan pages found: %ld\n", g->virtual_list->pages);

         g->virtual_list = g->virtual_list->next;
      }
   }
#endif
}

/*******************************************************************************
*
*   Allocate pages from the large blocks of virtual memory
*
*******************************************************************************/

void * _cs_gc_get_pages(cs_gc_struct * g, long n)
{
   void * pages;
   cs_gc_virtual_struct * vb;

   if (g->pages < n)
   {
      if (g->pages > 0)
         _cs_gc_free_pages(g, g->memmap_ptr, g->pages);

      _cs_gc_allocate_virtual(g, 1); 
   
      if (g->pages < n)
      {
         printf("Error: out of memory!\n");
         exit(1);
      }
   }

   pages = g->memmap_ptr;

   g->memmap_ptr += n*CS_PAGE_SIZE;
   g->pages -= n;

   /* mark pages active */
   vb = g->virtual_list;

   while (vb != NULL)
   {
      if (pages >= vb->start && pages < vb->end)
      {
         /* get first bit number */

         long i, bit = (pages - vb->start)/CS_PAGE_SIZE;
         long words, word = bit/CS_WORD_BITS;
         long bits1 = bit % CS_WORD_BITS;

         /* set n bits to 1 */

         if (bits1 > 0)
         {
            if (n >= CS_WORD_BITS - bits1)
            {
               vb->active[word++] |= ((~0UL) << bits1);
               bits1 = CS_WORD_BITS - bits1;
            } else
            {
               vb->active[word++] |= (((1UL << n) - 1) << bits1);
               bits1 = n;
            }

            n -= bits1;
         }

         words = n/CS_WORD_BITS;

         for (i = 0; i < words; i++)
            vb->active[word + i] = ~0UL;

         n -= words*CS_WORD_BITS;
         word += words;

         if (n > 0)
            vb->active[word] |= ((1UL << n) - 1);

         break;
      }

      vb = vb->next;
   }

   return pages;
}

void _cs_gc_free_pages(cs_gc_struct * g, void * ptr, long n)
{
   cs_gc_virtual_struct * vb = g->virtual_list;

   if (n == 0)
      return;

   if (munmap(ptr, n*CS_PAGE_SIZE) == -1)
   {
      printf("Warning: unable to free %ld pages!\n", n);
   }

   while (vb != NULL)
   {
      if (ptr >= vb->start && ptr < vb->end)
      {
         /* get first bit number */

         long i, bit = (ptr - vb->start)/CS_PAGE_SIZE;
         long words, word = bit/CS_WORD_BITS;
         long bits1 = bit % CS_WORD_BITS;
        
         /* set n bits to 0 */

         vb->pages -= n;
 
         if (bits1 > 0)
         {
            if (n >= CS_WORD_BITS - bits1)
            {
               vb->active[word++] &= ((1UL << bits1) - 1);
               bits1 = CS_WORD_BITS - bits1;
            } else
            {
               vb->active[word++] &= (((~0UL) << n) << bits1);
               bits1 = n;
            }

            n -= bits1;
         }

         words = n/CS_WORD_BITS;

         for (i = 0; i < words; i++)
            vb->active[word + i] = 0;

         n -= words*CS_WORD_BITS;
         word += words;

         if (n > 0)
            vb->active[word] &= ((~0UL) << n);

         /* clean up virtual block */

         if (vb->pages == 0)
         {
            long pages = (vb->end - vb->start)/CS_PAGE_SIZE;
            void * ptr = (void *) vb;
            
            pages = (pages + CS_PAGE_SIZE - 1)/CS_PAGE_SIZE;

            /* unlink virtual block */

            if (g->virtual_list == vb)
               g->virtual_list = vb->next;
            else
            {
               cs_gc_virtual_struct * vb2 = g->virtual_list;

               while (vb2->next != vb)
                  vb2 = vb2->next;
              
               vb2->next = vb->next;
            }
           
            if (munmap(ptr, (2*pages + 1)*CS_PAGE_SIZE) == -1)
            {
                printf("Warning: unable to free %ld pages!\n", n);
            }
         }
         
         break;
      }

      vb = vb->next;
   }
}

/*******************************************************************************
*
*   Allocate small fixed sized blocks from pools
*
*******************************************************************************/

