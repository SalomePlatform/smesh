# Copyright (C) 2007-2019  CEA/DEN, EDF R&D, OPEN CASCADE
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

SALOME_GENERATE_TESTS_ENVIRONMENT(_test_env)
SET(_test_helper ${CMAKE_SOURCE_DIR}/doc/salome/examples/testme.py)

# Tests from RESTRICTED repository -----------------------------------

SET(RESTRICTED_ROOT_DIR $ENV{RESTRICTED_ROOT_DIR} CACHE PATH "Path to the restricted repository")

IF(WITH_SHAPER_STUDY AND EXISTS ${RESTRICTED_ROOT_DIR})
  FILE(GLOB _restricted_tests "${RESTRICTED_ROOT_DIR}/SMESH/*.py")
  FOREACH(_test ${_restricted_tests})
    GET_FILENAME_COMPONENT(_test_name ${_test} NAME_WE)
    ADD_TEST(NAME ${_test_name}
             COMMAND ${PYTHON_EXECUTABLE} -B ${_test_helper} ${_test})
    SET_TESTS_PROPERTIES(${_test_name} PROPERTIES ENVIRONMENT "${_test_env}")
  ENDFOREACH()
ELSE()
  MESSAGE(WARNING "Tests from RESTRICTED repository aren't available")
ENDIF()

# --------------------------------------------------------------------
