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

# --- scripts ---

# scripts / static
SET(_bin_SCRIPTS
  smesh.py
  batchmode_smesh.py
  batchmode_mefisto.py
  ex00_all.py
  ex01_cube2build.py
  ex02_cube2primitive.py
  ex03_cube2partition.py
  ex04_cube5tetraHexa.py
  ex05_hole1build.py
  ex06_hole1boolean.py
  ex07_hole1partition.py
  ex08_hole2build.py
  ex09_grid4build.py
  ex10_grid4geometry.py
  ex11_grid3partition.py
  ex12_grid17partition.py
  ex13_hole1partial.py
  ex14_cyl1holed.py
  ex15_cyl2geometry.py
  ex16_cyl2complementary.py
  ex17_dome1.py
  ex18_dome2.py
  ex19_sphereINcube.py
  ex21_lamp.py
  ex24_cylinder.py
  ex29_refine.py
  ex30_tepal.py
  ex30_groupsOp.py
  ex31_dimGroup.py
  SMESH_test.py
  SMESH_test0.py
  SMESH_test1.py
  SMESH_test1_AndDisplay.py
  SMESH_test2.py
  SMESH_test3.py
  SMESH_test4.py
  SMESH_test5.py
  SMESH_mechanic.py
  SMESH_mechanic_tetra.py
  SMESH_mechanic_editor.py
  SMESH_mechanic_netgen.py
  SMESH_fixation.py
  SMESH_fixation_hexa.py
  SMESH_fixation_tetra.py
  SMESH_fixation_netgen.py
  SMESH_box_tetra.py
  SMESH_box2_tetra.py
  SMESH_box3_tetra.py
  SMESH_flight_skin.py
  SMESH_Partition1_tetra.py
  SMESH_controls.py
  SMESH_freebord.py
  SMESH_blocks.py
  SMESH_BelongToGeom.py
  SMESH_GroupFromGeom2.py
  SMESH_box.py
  SMESH_demo_hexa2_upd.py
  SMESH_hexaedre.py
  SMESH_Sphere.py
  SMESH_GroupFromGeom.py
  SMESH_Nut.py
  SMESH_GroupLyingOnGeom.py
  SMESH_AdvancedEditor.py
  SMESH_BuildCompound.py
  PAL_MESH_041_mesh.py
  PAL_MESH_043_2D.py
  PAL_MESH_043_3D.py
  SMESH_reg.py
  smesh_selection.py
)

SET(smesh_SCRIPTS
  smeshBuilder.py
  smesh_algorithm.py
)

SET(StdMeshers_SCRIPTS
  __init__.py
  StdMeshersBuilder.py
)
# --- rules ---
SALOME_INSTALL_SCRIPTS("${_bin_SCRIPTS}" ${SALOME_INSTALL_SCRIPT_PYTHON} DEF_PERMS)
SALOME_INSTALL_SCRIPTS("${smesh_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/smesh DEF_PERMS)
SALOME_INSTALL_SCRIPTS("${StdMeshers_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/StdMeshers DEF_PERMS)
SALOME_INSTALL_SCRIPTS(SMESH_shared_modules.py ${SALOME_INSTALL_PYTHON}/shared_modules DEF_PERMS)
