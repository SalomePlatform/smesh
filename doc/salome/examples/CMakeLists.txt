# Copyright (C) 2012-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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

SET(TEST_REINIT_SALOME "False")
SALOME_CONFIGURE_FILE(tests.py.in tests.py)

SALOME_GENERATE_TESTS_ENVIRONMENT(tests_env)

IF(SMESH_JOIN_TESTS)
  ADD_TEST(NAME SMESH_examples COMMAND ${PYTHON_EXECUTABLE} -B ${CMAKE_SOURCE_DIR}/doc/salome/examples/testme.py tests.py)
ELSE(SMESH_JOIN_TESTS)
  FOREACH(test ${GOOD_TESTS})
    GET_FILENAME_COMPONENT(testname ${test} NAME_WE)
    ADD_TEST(NAME ${testname}
             COMMAND ${PYTHON_EXECUTABLE} -B ${CMAKE_SOURCE_DIR}/doc/salome/examples/testme.py ${CMAKE_CURRENT_SOURCE_DIR}/${test})
    SET_TESTS_PROPERTIES(${testname} PROPERTIES ENVIRONMENT "${tests_env}")
  ENDFOREACH()
ENDIF(SMESH_JOIN_TESTS)

# install Python scripts
SALOME_INSTALL_SCRIPTS("${EXAMPLES_TESTS}" ${SALOME_INSTALL_DOC}/examples/SMESH)

# Application tests

SET(TEST_INSTALL_DIRECTORY ${SALOME_INSTALL_SCRIPT_SCRIPTS}/test)
INSTALL(FILES ${GOOD_TESTS} ${BAD_TESTS} ${SESSION_FREE_TESTS} DESTINATION ${TEST_INSTALL_DIRECTORY})

INSTALL(FILES CTestTestfileInstall.cmake
        DESTINATION ${TEST_INSTALL_DIRECTORY}
        RENAME CTestTestfile.cmake)
INSTALL(FILES tests.set DESTINATION ${TEST_INSTALL_DIRECTORY})
INSTALL(DIRECTORY MGAdapt_med_files DESTINATION ${TEST_INSTALL_DIRECTORY})
