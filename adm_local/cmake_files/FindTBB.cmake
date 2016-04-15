# - Find TBB
# Sets the following variables:
#   TBB_INCLUDE_DIRS - path to the TBB include directory
#   TBB_LIBRARIES    - path to the TBB libraries to be linked against
#

#########################################################################
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# ------

MESSAGE(STATUS "Check for TBB ...")

# ------

SET(TBB_ROOT_DIR $ENV{TBB_ROOT_DIR})

IF(TBB_ROOT_DIR)
  LIST(APPEND CMAKE_PREFIX_PATH "${TBB_ROOT_DIR}")
ENDIF(TBB_ROOT_DIR)

FIND_PATH(TBB_INCLUDE_DIRS tbb/tbb.h PATH_SUFFIXES include)

IF(MACHINE_IS_64)
  SET(PLT_SUFFIX intel64)
ELSE(MACHINE_IS_64)
  SET(PLT_SUFFIX ia32)
ENDIF(MACHINE_IS_64)

include(CheckCXXSourceCompiles)

FOREACH(_kernel cc4.1.0_libc2.4_kernel2.6.16.21 gcc4.4 gcc4.1)

  FIND_LIBRARY(_tbb_library_tbb${_kernel}       NAMES tbb       PATH_SUFFIXES lib/${PLT_SUFFIX}/${_kernel})
  FIND_LIBRARY(_tbb_library_tbbmalloc${_kernel} NAMES tbbmalloc PATH_SUFFIXES lib/${PLT_SUFFIX}/${_kernel})

  SET(_tbb_libraries${_kernel} ${_tbb_library_tbb${_kernel}} ${_tbb_library_tbbmalloc${_kernel}})

  IF(_tbb_libraries${_kernel})

    SET(CMAKE_REQUIRED_INCLUDES_SAVE  ${CMAKE_REQUIRED_INCLUDES})
    SET(CMAKE_REQUIRED_LIBRARIES_SAVE ${CMAKE_REQUIRED_LIBRARIES})
    SET(CMAKE_REQUIRED_INCLUDES  "${CMAKE_REQUIRED_INCLUDES} ${TBB_INCLUDE_DIRS}")
    SET(CMAKE_REQUIRED_LIBRARIES "${_tbb_libraries${_kernel}}")

    CHECK_CXX_SOURCE_COMPILES("
      #include <tbb/tbb.h>
      using namespace tbb;
      size_t testme(size_t n)
      {
        return n*n;
      }
      int main(int argc, char* argv[])
      {
        parallel_for<size_t>( 1, 10, 1, testme );
      }
      "
      _tbb_link_ok${_kernel}
      )
    
    SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})
    SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_SAVE})

    IF(_tbb_link_ok${_kernel})
      SET(_tbb_link_ok ${_tbb_link_ok${_kernel}})
      SET(TBB_LIBRARY_tbb       ${_tbb_library_tbb${_kernel}}       CACHE FILEPATH "Path to a library")
      SET(TBB_LIBRARY_tbbmalloc ${_tbb_library_tbbmalloc${_kernel}} CACHE FILEPATH "Path to a library")
      SET(TBB_LIBRARIES ${TBB_LIBRARY_tbb} ${TBB_LIBRARY_tbbmalloc})
    ENDIF()

    UNSET(_tbb_link_ok${_kernel} CACHE)

  ENDIF(_tbb_libraries${_kernel})

  UNSET(_tbb_library_tbb${_kernel} CACHE)
  UNSET(_tbb_library_tbbmalloc${_kernel} CACHE)

  IF(_tbb_link_ok)
    BREAK()
  ENDIF()

ENDFOREACH()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TBB REQUIRED_VARS TBB_INCLUDE_DIRS TBB_LIBRARY_tbb TBB_LIBRARY_tbbmalloc)
