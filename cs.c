#include <stdio.h>
#include "gc.h"

void test_gc(cs_gc_struct * g)
{
   long i;
   void * block[10];

   printf("gc...");
   fflush(stdout);

   for (i = 5; i < 10; i++)
      block[i] = _cs_gc_get_pages(g, 100);

   for (i = 0; i < 1000000; i++)
   {
      _cs_gc_free_pages(g, block[(i + 5)%10], 100);
      block[i%10] = _cs_gc_get_pages(g, 100);
   }
   
   for (i = 5; i < 10; i++)
      _cs_gc_free_pages(g, block[i], 100);

   printf("PASSED\n");
}

int main(void)
{
   cs_gc_struct g;

   _cs_gc_init(&g);
   
   test_gc(&g);

   _cs_gc_clear(&g);

   return 0;
}

