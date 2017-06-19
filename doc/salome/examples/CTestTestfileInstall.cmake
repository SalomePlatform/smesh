# Copyright (C) 2015-2017  CEA/DEN, EDF R&D, OPEN CASCADE
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

INCLUDE(tests.set)

SET(SALOME_TEST_DRIVER "$ENV{ABSOLUTE_APPLI_PATH}/bin/salome/appliskel/salome_test_driver.py")
SET(COMPONENT_NAME SMESH)
SET(TIMEOUT        300)

FOREACH(tfile ${GOOD_TESTS})
  GET_FILENAME_COMPONENT(BASE_NAME ${tfile} NAME_WE)
  SET(TEST_NAME SMESH_${BASE_NAME})
  ADD_TEST(${TEST_NAME} python ${SALOME_TEST_DRIVER} ${TIMEOUT} ${tfile})
  SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES LABELS "${COMPONENT_NAME}")
ENDFOREACH()
