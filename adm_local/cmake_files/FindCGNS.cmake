# - Find CGNS
# Sets the following variables:
#   CGNS_INCLUDE_DIRS - path to the CGNS include directory
#   CGNS_LIBRARIES    - path to the CGNS libraries to be linked against
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

MESSAGE(STATUS "Check for CGNS ...")

# ------

SET(CGNS_ROOT_DIR $ENV{CGN_ROOT_DIR} CACHE PATH "Path to the CGNS.")

IF(CGNS_ROOT_DIR)
 LIST(APPEND CMAKE_PREFIX_PATH "${CGNS_ROOT_DIR}")
ENDIF(CGNS_ROOT_DIR)

FIND_PATH(CGNS_INCLUDE_DIRS cgnslib.h)
FIND_LIBRARY(CGNS_LIBRARIES NAMES cgns)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CGNS REQUIRED_VARS CGNS_INCLUDE_DIRS CGNS_LIBRARIES)
