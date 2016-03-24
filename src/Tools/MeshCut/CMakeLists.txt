# Copyright (C) 2012-2016  EDF R&D
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

IF(SALOME_BUILD_GUI)
  INCLUDE(UsePyQt)
ENDIF(SALOME_BUILD_GUI)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${MEDFILE_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${HDF5_LIBRARIES}
  ${MEDFILE_C_LIBRARIES}
)

# --- scripts ---

# scripts / static
SET(plugin_SCRIPTS
  meshcut_plugin.py
)

# bin programs
SET(MeshCut_SOURCES
  MeshCut_Globals.hxx
  MeshCut_Carre.hxx
  MeshCut_Carre.cxx
  MeshCut_Cube.hxx
  MeshCut_Cube.cxx
  MeshCut_Maillage.hxx
  MeshCut_Maillage.cxx
  MeshCut_Fonctions.hxx
  MeshCut_Fonctions.cxx
  MeshCut_Utils.hxx
  MeshCut_Utils.cxx
  MeshCut_Cas.hxx
  MeshCut_Cas.cxx
  MeshCut_DC.cxx
)

IF(SALOME_BUILD_GUI)
  # --- resources ---

  # uic files / to be processed by pyuic
  SET(_pyuic_files
    MeshCutDialog.ui
  )

  # scripts / pyuic wrappings
  PYQT_WRAP_UIC(_pyuic_SCRIPTS ${_pyuic_files})

ENDIF(SALOME_BUILD_GUI)

# --- rules ---

SALOME_INSTALL_SCRIPTS("${plugin_SCRIPTS}" ${SALOME_SMESH_INSTALL_PLUGINS})

ADD_EXECUTABLE(MeshCut ${MeshCut_SOURCES})
TARGET_LINK_LIBRARIES(MeshCut ${_link_LIBRARIES})
INSTALL(TARGETS MeshCut EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_BINS})

IF(SALOME_BUILD_GUI)
  INSTALL(FILES ${_pyuic_SCRIPTS} DESTINATION ${SALOME_INSTALL_BINS})
ENDIF(SALOME_BUILD_GUI)
