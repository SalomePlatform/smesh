# Copyright (C) 2015-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

SET(SALOME_TEST_DRIVER "$ENV{ABSOLUTE_APPLI_PATH}/bin/salome/appliskel/salome_test_driver.py")
SET(COMPONENT_NAME SMESH)
SET(TIMEOUT        300)

SET(GOOD_TESTS
  cartesian_algo
  creating_meshes_ex02
  creating_meshes_ex04
  creating_meshes_ex06
  creating_meshes_ex07
  creating_meshes_ex08
  defining_hypotheses_ex01
  defining_hypotheses_ex02
  defining_hypotheses_ex03
  defining_hypotheses_ex04
  defining_hypotheses_ex05
  defining_hypotheses_ex07
  defining_hypotheses_ex08
  defining_hypotheses_ex10
  defining_hypotheses_ex11
  defining_hypotheses_ex12
  defining_hypotheses_ex13
  defining_hypotheses_ex14
  defining_hypotheses_ex15
  defining_hypotheses_ex16
  defining_hypotheses_adaptive1d
  filters_ex01
  filters_ex03
  filters_ex04
  filters_ex05
  filters_ex06
  filters_ex07
  filters_ex09
  filters_ex10
  filters_ex11
  filters_ex12
  filters_ex13
  filters_ex14
  filters_ex15
  filters_ex16
  filters_ex17
  filters_ex18
  filters_ex19
  filters_ex20
  filters_ex21
  filters_ex22
  filters_ex26
  filters_ex27
  filters_ex28
  filters_ex29
  filters_ex30
  filters_ex31
  filters_ex33
  filters_ex34
  filters_ex36
  grouping_elements_ex01
  grouping_elements_ex02
  grouping_elements_ex03
  grouping_elements_ex04
  grouping_elements_ex05
  grouping_elements_ex06
  grouping_elements_ex07
  grouping_elements_ex08
  measurements_ex01
  measurements_ex02
  modifying_meshes_ex01
  modifying_meshes_ex02
  modifying_meshes_ex03
  modifying_meshes_ex04
  modifying_meshes_ex05
  modifying_meshes_ex06
  modifying_meshes_ex07
  modifying_meshes_ex08
  modifying_meshes_ex09
  modifying_meshes_ex10
  modifying_meshes_ex11
  modifying_meshes_ex12
  modifying_meshes_ex13
  modifying_meshes_ex14
  modifying_meshes_ex15
  modifying_meshes_ex16
  modifying_meshes_ex17
  modifying_meshes_ex18
  modifying_meshes_ex19
  modifying_meshes_ex20
  modifying_meshes_ex21
  modifying_meshes_ex22
  modifying_meshes_ex23
  modifying_meshes_ex24
  modifying_meshes_ex25
  prism_3d_algo
  quality_controls_ex01
  quality_controls_ex02
  quality_controls_ex03
  quality_controls_ex04
  quality_controls_ex05
  quality_controls_ex07
  quality_controls_ex08
  quality_controls_ex09
  quality_controls_ex10
  quality_controls_ex11
  quality_controls_ex12
  quality_controls_ex13
  quality_controls_ex14
  quality_controls_ex15
  quality_controls_ex16
  quality_controls_ex17
  quality_controls_ex18
  quality_controls_ex19
  transforming_meshes_ex01
  transforming_meshes_ex02
  transforming_meshes_ex03
  transforming_meshes_ex04
  transforming_meshes_ex05
  transforming_meshes_ex06
  transforming_meshes_ex07
  transforming_meshes_ex08
  transforming_meshes_ex09
  transforming_meshes_ex10
  transforming_meshes_ex11
  transforming_meshes_ex12
  transforming_meshes_ex13
  use_existing_faces
  viewing_meshes_ex02
)

FOREACH(tfile ${GOOD_TESTS})
  SET(TEST_NAME SMESH_${tfile})
  ADD_TEST(${TEST_NAME} python ${SALOME_TEST_DRIVER} ${TIMEOUT} ${tfile}.py)
  SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES LABELS "${COMPONENT_NAME}")
ENDFOREACH()
