# - Find F2C
# Sets the following variables:
#   f2c_INCLUDE_DIRS - path to the f2c include directory
#   f2c_LIBRARIES    - path to the f2c libraries to be linked against
#   f2c_GENERATOR    - f2c executable 
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

MESSAGE(STATUS "Check for f2c ...")

# ------

SET(f2c_ROOT_DIR $ENV{F2C_ROOT_DIR})

IF(f2c_ROOT_DIR)
  LIST(APPEND CMAKE_PREFIX_PATH "${f2c_ROOT_DIR}")
ENDIF(f2c_ROOT_DIR)

FIND_PATH(f2c_INCLUDE_DIRS F2c.h)
FIND_LIBRARY(f2c_LIBRARY_LIBF77 NAMES LIBF77)
FIND_LIBRARY(f2c_LIBRARY_LIBI77 NAMES LIBI77)

SET(f2c_LIBRARIES
  ${f2c_LIBRARY_LIBF77}
  ${f2c_LIBRARY_LIBI77}
)

FIND_PROGRAM(f2c_GENERATOR F2c)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(f2c REQUIRED_VARS f2c_INCLUDE_DIRS f2c_LIBRARIES f2c_GENERATOR)