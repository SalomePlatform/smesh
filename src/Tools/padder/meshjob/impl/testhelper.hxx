// Copyright (C) 2011-2016  EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// Authors : Guillaume Boulant (EDF) - 01/03/2011

#ifndef __TESTHELPER_HXX__
#define __TESTHELPER_HXX__

// >>>
// WARN: this file is DEPRECATED and/or should be used for test
// purpose only. The PADDER configuration is now read in a
// configuration file (padder.cfg).
// <<<

#include <stdlib.h> // Standard C include (for getenv)
#include <unistd.h>
#include <string>

/*!
 * This function returns the module SMESH installation root directory
 * as a string.
 */
static std::string SMESH_ROOT_DIR() {
  static std::string * smesh_root_dir;
  if ( smesh_root_dir == NULL ) {
    char * SMESH_ROOT_DIR = getenv("SMESH_ROOT_DIR");
    if ( SMESH_ROOT_DIR == NULL ) {
      smesh_root_dir = new std::string("<path_undefined>");
    }
    else {
      smesh_root_dir = new std::string(SMESH_ROOT_DIR);
    }
  }
  return *smesh_root_dir;
}

/*! Relative path of the directory containing data and exe for tests */
static std::string PADDEREXE_RPATH("/share/salome/resources/smesh/padderexe");
/*! Absolute path of the directory containing data and exe for tests */
static std::string PADDEREXE_APATH(SMESH_ROOT_DIR()+PADDEREXE_RPATH);
/*! Absolute path of the exe shell script for tests */
static std::string PADDEREXE_SCRIPT_FILENAME(PADDEREXE_APATH+"/padder.sh");


static int testssh_using_system() {
  const char * cmd = "/usr/bin/ssh claui2p1 -l boulant 'cd /tmp && ./runCommand_padder_Mon_Feb_28_14_28_36_2011.sh'";
  int result = system(cmd);
  return result;
}

#endif // __TESTHELPER_HXX__
