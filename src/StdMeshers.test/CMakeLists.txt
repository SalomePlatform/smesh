# Copyright (C) 2012-2026  CEA, EDF, OPEN CASCADE
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

SALOME_GENERATE_TESTS_ENVIRONMENT(_test_env)

SET(TEST_INSTALL_DIRECTORY ${SMESH_TEST_DIR}/other)

INCLUDE_DIRECTORIES(
  ${OpenCASCADE_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${CPPUNIT_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/StdMeshers
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_SOURCE_DIR}/src/SMESH
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/Controls
)
# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${OpenCASCADE_DEFINITIONS}
  ${BOOST_DEFINITIONS}
  ${CPPUNIT_DEFINITIONS}
)

IF(SALOME_SMESH_USE_TBB)
  SET(TBB_LIBS ${TBB_LIBRARIES})
ENDIF(SALOME_SMESH_USE_TBB)

FOREACH(_test ${UNIT_TESTS})
  GET_FILENAME_COMPONENT(testname ${_test} NAME_WE)
  SET(testname "TESTS_${testname}")

  add_executable(${_test} ${_test}.cxx)
  target_link_libraries(${_test} StdMeshers ${CPPUNIT_LIBRARIES} )

  ADD_TEST(NAME ${testname}
           COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/${_test} )
  SET_TESTS_PROPERTIES(${testname} PROPERTIES ENVIRONMENT "${tests_env}" LABELS "tests")
ENDFOREACH()

IF(WIN32)
  FOREACH(_test ${UNIT_TESTS})
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${_test}${CMAKE_EXECUTABLE_SUFFIX} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ DESTINATION ${TEST_INSTALL_DIRECTORY})
  ENDFOREACH()
ELSE()
  FOREACH(_test ${UNIT_TESTS})
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_test} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ DESTINATION ${TEST_INSTALL_DIRECTORY})
  ENDFOREACH()
ENDIF(WIN32)

