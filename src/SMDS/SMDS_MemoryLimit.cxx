// File      : SMDS_MemoryLimit.cxx
// Created   : Fri Sep 21 17:16:42 2007
// Author    : Edward AGAPOV (eap)

// Executable to find out a lower RAM limit (MB), i.e. at what size of freeRAM
// reported by sysinfo, no more memory can be allocated.
// This is not done inside a function of SALOME because allocated memory is not returned
// to the system

#ifndef WIN32
#include <sys/sysinfo.h>
#endif

#ifdef _DEBUG_
#include <iostream>
#endif

int main (int argc, char ** argv) {
#ifndef WIN32
  struct sysinfo si;
  int err = sysinfo( &si );
  if ( err )
    return -1;
  unsigned long freeRamKb = ( si.freeram  * si.mem_unit ) / 1024;

  const unsigned long stepKb = 8; // less nb leads to hung up on Mandriva2006 without swap
  // (other platforms not tested w/o swap) 

  unsigned long nbSteps = freeRamKb / stepKb * 2;
  try {
    while ( nbSteps-- ) {
      new char[stepKb*1024];
      err = sysinfo( &si );
      if ( !err )
        freeRamKb = ( si.freeram  * si.mem_unit ) / 1024;
    }
  } catch (...) {}

// #ifdef _DEBUG_
//   std::cout << freeRamKb / 1024 << std::endl;
// #endif
  return freeRamKb / 1024;

#endif

  return -1;
}
