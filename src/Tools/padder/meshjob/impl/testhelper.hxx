#ifndef __TESTHELPER_HXX__
#define __TESTHELPER_HXX__

#include <stdlib.h> // Standard C include (for getenv)
#include <unistd.h>
#include <string>

/*!
 * This function returns the module SPADDER installation root
 * directory as a string.
 */
static std::string SPADDER_ROOT_DIR() {
  static std::string * spadder_root_dir;
  if ( spadder_root_dir == NULL ) {
    char * SPADDER_ROOT_DIR = getenv("SPADDER_ROOT_DIR");
    spadder_root_dir = new std::string(SPADDER_ROOT_DIR);
  }
  return *spadder_root_dir;
}

/*! Relative path of the directory containing data and exe for tests */
static std::string PADDEREXE_RPATH("/share/salome/resources/spadder/padderexe");
/*! Absolute path of the directory containing data and exe for tests */
static std::string PADDEREXE_APATH(SPADDER_ROOT_DIR()+PADDEREXE_RPATH);
/*! Absolute path of the exe shell script for tests */
static std::string PADDEREXE_SCRIPT_FILENAME(PADDEREXE_APATH+"/padder.sh");


static int testssh_using_system() {
  const char * cmd = "/usr/bin/ssh claui2p1 -l boulant 'cd /tmp && ./runCommand_padder_Mon_Feb_28_14_28_36_2011.sh'";
  int result = system(cmd);
  return result;
}

#endif // __TESTHELPER_HXX__
