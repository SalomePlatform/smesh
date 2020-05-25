# Copyright (C) 2012-2020  CEA/DEN, EDF R&D
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

INCLUDE(UseQtExt)
INCLUDE(UsePyQt)

# --- options ---

# additional include directories
INCLUDE_DIRECTORIES(
  ${QT_INCLUDES}
  ${OpenCASCADE_INCLUDE_DIR}
  ${PYTHON_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
  ${PROJECT_BINARY_DIR}/adm_local/unix
  ${PROJECT_SOURCE_DIR}/src/ADAPT
  ${PROJECT_SOURCE_DIR}/src/ADAPT_I
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
  ${OpenCASCADE_DEFINITIONS}
  ${KERNEL_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${MEDFILE_C_LIBRARIES}
  ${KERNEL_SalomeLifeCycleCORBA}
  ${KERNEL_SalomeDS}
  ${GUI_SalomeApp}
  ${SMESH_SalomeIDLSMESH}
  ADAPTEngine
)

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  ADAPT_msg_en.ts
  ADAPT_msg_fr.ts
  ADAPT_msg_ja.ts
)

# resource files / to be processed by uic
SET(_uic_FILES
  CreateBoundaryCAO.ui
  CreateBoundaryAn.ui
  CreateBoundaryDi.ui
  CreateCase.ui
  CreateHypothesis.ui
  CreateIteration.ui
  CreateListGroup.ui
  CreateYACS.ui
  CreateZone.ui
  EditFile.ui
  IterInfo.ui
  MeshInfo.ui
  PursueIteration.ui
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  ADAPTGUI.h
  MonCreateBoundaryCAO.h
  MonCreateBoundaryAn.h
  MonCreateBoundaryDi.h
  MonEditBoundaryCAO.h
  MonEditBoundaryAn.h
  MonEditBoundaryDi.h
  MonCreateCase.h
  MonEditCase.h
  MonCreateZone.h
  MonEditZone.h
  MonCreateHypothesis.h
  MonEditHypothesis.h
  MonCreateListGroup.h
  MonCreateListGroupCAO.h
  MonEditListGroup.h
  MonEditListGroupCAO.h
  MonCreateIteration.h
  MonEditIteration.h
  MonPursueIteration.h
  MonMeshInfo.h
  MonIterInfo.h
  MonCreateYACS.h
  MonEditYACS.h
  MonEditFile.h
)

# header files / uic wrappings
QT_WRAP_UIC(_uic_HEADERS ${_uic_FILES})

# header files / static
SET(_other_HEADERS
  HOMARDGUI_Utils.h
  HomardQtCommun.h
  HOMARDGUI_Exports.hxx
)

# header files / to install
SET(ADAPT_HEADERS ${_other_HEADERS} ${_moc_HEADERS} ${_uic_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  ADAPTGUI.cxx
  HOMARDGUI_Utils.cxx
  MonCreateBoundaryCAO.cxx
  MonCreateBoundaryAn.cxx
  MonCreateBoundaryDi.cxx
  MonEditBoundaryCAO.cxx
  MonEditBoundaryAn.cxx
  MonEditBoundaryDi.cxx
  MonCreateCase.cxx
  MonEditCase.cxx
  MonCreateHypothesis.cxx
  MonEditHypothesis.cxx
  MonCreateListGroup.cxx
  MonCreateListGroupCAO.cxx
  MonEditListGroup.cxx
  MonEditListGroupCAO.cxx
  MonCreateIteration.cxx
  MonEditIteration.cxx
  MonPursueIteration.cxx
  MonCreateZone.cxx
  MonEditZone.cxx
  MonMeshInfo.cxx
  MonIterInfo.cxx
  MonCreateYACS.cxx
  MonEditYACS.cxx
  MonEditFile.cxx
  HomardQtCommun.cxx
)

# sources / to compile
SET(ADAPT_SOURCES ${_other_SOURCES} ${_moc_SOURCES} ${_uic_HEADERS})

# --- rules ---

ADD_LIBRARY(ADAPT ${ADAPT_SOURCES})
TARGET_LINK_LIBRARIES(ADAPT ${_link_LIBRARIES} )
INSTALL(TARGETS ADAPT EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${ADAPT_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_ADAPT_INSTALL_RES_DATA}")
