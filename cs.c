#include <stdio.h>
#include "gc.h"

void test_gc(void)
{
   printf("gc...");
   fflush(stdout);

   printf("PASSED\n");
}

int main(void)
{
   cs_gc_struct g;

   _cs_gc_init(&g);
   
   test_gc();

   _cs_gc_clear(&g);

   return 0;
}

