# Copyright (C) 2014-2016  EDF R&D
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

SET(MACMESH_INSTALL_PY ${SALOME_SMESH_INSTALL_PLUGINS}/MacMesh)
SET(MACMESH_INSTALL_DOC ${SALOME_INSTALL_DOC}/gui/SMESH/MacMesh)

SALOME_CONFIGURE_FILE(Doc/index.html.in index.html)
SALOME_CONFIGURE_FILE(Example/PressureValve.py.in PressureValve.py)

SET(plugin_DOC_FILES
  Doc/MacMesh.v.10avr.pdf
  ${CMAKE_CURRENT_BINARY_DIR}/index.html
  Doc/snap.jpg
  )

IF(SALOME_BUILD_DOC)
  INSTALL(FILES ${plugin_DOC_FILES} DESTINATION ${MACMESH_INSTALL_DOC})
ENDIF(SALOME_BUILD_DOC)

# --- scripts ---

# scripts / static
SET(plugin_SCRIPTS
  MacMesh/Alarms.py
  MacMesh/CentralUnrefine.py
  MacMesh/CompositeBox.py
  MacMesh/CompositeBoxF.py
  MacMesh/Config.py
  MacMesh/CutnGroup.py
  MacMesh/Cylinder.py
  MacMesh/GenFunctions.py
  MacMesh/MacObject.py
  MacMesh/PublishGroups.py
  MacMesh/SharpAngle.py
)
SET(sample_SCRIPT
  ${CMAKE_CURRENT_BINARY_DIR}/PressureValve.py
)
# --- rules ---

SALOME_INSTALL_SCRIPTS("${plugin_SCRIPTS}" ${MACMESH_INSTALL_PY})
SALOME_INSTALL_SCRIPTS("${sample_SCRIPT}" ${SALOME_INSTALL_SCRIPT_PYTHON} DEF_PERMS)

SET(testname MacMesh_Example_PressureValve)
SALOME_GENERATE_TESTS_ENVIRONMENT(tests_env)
ADD_TEST(
  NAME ${testname}
  COMMAND ${PYTHON_EXECUTABLE} -B ${CMAKE_SOURCE_DIR}/doc/salome/examples/testme.py ${sample_SCRIPT})
SET_TESTS_PROPERTIES(${testname} PROPERTIES ENVIRONMENT "${tests_env}")

