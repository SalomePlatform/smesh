# Copyright (C) 2012-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

# examples that cant be used for testing because they use external mesher plug-ins
SET(BAD_TESTS
  3dmesh.py
  a3DmeshOnModified2Dmesh.py
  creating_meshes_ex01.py
  creating_meshes_ex03.py
  creating_meshes_ex05.py
  defining_hypotheses_ex06.py
  defining_hypotheses_ex09.py
  defining_hypotheses_ex17.py
  filters_ex02.py
  filters_ex08.py
  filters_ex23.py
  filters_ex24.py
  filters_ex25.py
  filters_ex32.py
  filters_ex35.py
  generate_flat_elements.py
  modifying_meshes_ex26.py
  notebook_smesh.py
  quality_controls_ex06.py
  quality_controls_ex20.py
  quality_controls_ex21.py
  quality_controls_ex22.py
  viewing_meshes_ex01.py
  )

SET(GOOD_TESTS
  cartesian_algo.py
  creating_meshes_ex02.py
  creating_meshes_ex04.py
  creating_meshes_ex06.py
  creating_meshes_ex07.py
  creating_meshes_ex08.py
  defining_hypotheses_ex01.py
  defining_hypotheses_ex02.py
  defining_hypotheses_ex03.py
  defining_hypotheses_ex04.py
  defining_hypotheses_ex05.py
  defining_hypotheses_ex07.py
  defining_hypotheses_ex08.py
  defining_hypotheses_ex10.py
  defining_hypotheses_ex11.py
  defining_hypotheses_ex12.py
  defining_hypotheses_ex13.py
  defining_hypotheses_ex14.py
  defining_hypotheses_ex15.py
  defining_hypotheses_ex16.py
  defining_hypotheses_adaptive1d.py
  filters_ex01.py
  filters_ex03.py
  filters_ex04.py
  filters_ex05.py
  filters_ex06.py
  filters_ex07.py
  filters_ex09.py
  filters_ex10.py
  filters_ex11.py
  filters_ex12.py
  filters_ex13.py
  filters_ex14.py
  filters_ex15.py
  filters_ex16.py
  filters_ex17.py
  filters_ex18.py
  filters_ex19.py
  filters_ex20.py
  filters_ex21.py
  filters_ex22.py
  filters_ex26.py
  filters_ex27.py
  filters_ex28.py
  filters_ex29.py
  filters_ex30.py
  filters_ex31.py
  filters_ex33.py
  filters_ex34.py
  filters_ex36.py
  filters_ex37.py
  filters_ex38.py
  filters_ex39.py
  filters_node_nb_conn.py
  filters_belong2group.py
  grouping_elements_ex01.py
  grouping_elements_ex02.py
  grouping_elements_ex03.py
  grouping_elements_ex04.py
  grouping_elements_ex05.py
  grouping_elements_ex06.py
  grouping_elements_ex07.py
  grouping_elements_ex08.py
  measurements_ex01.py
  measurements_ex02.py
  modifying_meshes_ex01.py
  modifying_meshes_ex02.py
  modifying_meshes_ex03.py
  modifying_meshes_ex04.py
  modifying_meshes_ex05.py
  modifying_meshes_ex06.py
  modifying_meshes_ex07.py
  modifying_meshes_ex08.py
  modifying_meshes_ex09.py
  modifying_meshes_ex10.py
  modifying_meshes_ex11.py
  modifying_meshes_ex12.py
  modifying_meshes_ex13.py
  modifying_meshes_ex14.py
  modifying_meshes_ex15.py
  modifying_meshes_ex16.py
  modifying_meshes_ex17.py
  modifying_meshes_ex18.py
  modifying_meshes_ex19.py
  modifying_meshes_ex20.py
  modifying_meshes_ex21.py
  modifying_meshes_ex22.py
  modifying_meshes_ex23.py
  modifying_meshes_ex24.py
  modifying_meshes_ex25.py
  prism_3d_algo.py
  quality_controls_ex01.py
  quality_controls_ex02.py
  quality_controls_ex03.py
  quality_controls_ex04.py
  quality_controls_ex05.py
  quality_controls_ex07.py
  quality_controls_ex08.py
  quality_controls_ex09.py
  quality_controls_ex10.py
  quality_controls_ex11.py
  quality_controls_ex12.py
  quality_controls_ex13.py
  quality_controls_ex14.py
  quality_controls_ex15.py
  quality_controls_ex16.py
  quality_controls_ex17.py
  quality_controls_ex18.py
  quality_controls_ex19.py
  transforming_meshes_ex01.py
  transforming_meshes_ex02.py
  transforming_meshes_ex03.py
  transforming_meshes_ex04.py
  transforming_meshes_ex05.py
  transforming_meshes_ex06.py
  transforming_meshes_ex07.py
  transforming_meshes_ex08.py
  transforming_meshes_ex09.py
  transforming_meshes_ex10.py
  transforming_meshes_ex11.py
  transforming_meshes_ex12.py
  transforming_meshes_ex13.py
  use_existing_faces.py
  viewing_meshes_ex02.py
  split_biquad.py
)

SET(EXAMPLES_TESTS ${BAD_TESTS} ${GOOD_TESTS} testme.py)

SALOME_GENERATE_TESTS_ENVIRONMENT(tests_env)

FOREACH(test ${GOOD_TESTS})
  GET_FILENAME_COMPONENT(testname ${test} NAME_WE)
  ADD_TEST(NAME ${testname}
           COMMAND ${PYTHON_EXECUTABLE} -B ${CMAKE_SOURCE_DIR}/doc/salome/examples/testme.py ${CMAKE_CURRENT_SOURCE_DIR}/${test})
  SET_TESTS_PROPERTIES(${testname} PROPERTIES ENVIRONMENT "${tests_env}")
ENDFOREACH()

# install Python scripts
SALOME_INSTALL_SCRIPTS("${EXAMPLES_TESTS}" ${SALOME_INSTALL_DOC}/examples/SMESH)

# Application tests

SET(TEST_INSTALL_DIRECTORY ${SALOME_INSTALL_SCRIPT_SCRIPTS}/test)
INSTALL(FILES ${GOOD_TESTS} DESTINATION ${TEST_INSTALL_DIRECTORY})

INSTALL(FILES CTestTestfileInstall.cmake
        DESTINATION ${TEST_INSTALL_DIRECTORY}
        RENAME CTestTestfile.cmake)
