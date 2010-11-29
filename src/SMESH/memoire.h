#ifndef _MEMOIRE_H_
#define _MEMOIRE_H_

#include <malloc.h>
#include <iostream>

void memostat(const char* f, int l);

void memostat(const char* f, int l)
{
  /*  struct mallinfo mem = mallinfo(); */
  /*  std::cerr << f << ":"<< l << " " << mem.arena << " " << mem.ordblks << " " << mem.hblks << " " << mem.hblkhd << " "  << mem.uordblks << " "  << mem.fordblks << " " << mem.keepcost << std::endl; */
  std::cerr << f << ":" << l << " --------------------------" << std::endl;
  malloc_stats();
  std::cerr << f << ":" << l << " --------------------------" << std::endl;
}

#define MEMOSTAT memostat( __FILE__, __LINE__ )

#endif
