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

INCLUDE(UseQtExt)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${QT_INCLUDES}
  ${VTK_INCLUDE_DIRS}
  ${OGL_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${QWT_INCLUDE_DIR}
  ${GEOM_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${HDF5_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/OBJECT
  ${PROJECT_SOURCE_DIR}/src/SMESHFiltersSelection
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_SOURCE_DIR}/src/Controls
  ${PROJECT_SOURCE_DIR}/src/SMESHClient
  ${PROJECT_SOURCE_DIR}/src/MEDWrapper/Base
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
)

# To avoid GetObject macro definition on WIN32
IF(WIN32)
  ADD_DEFINITIONS(-DNOGDI)
ENDIF(WIN32)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKV3d}
  ${GUI_SalomePrs}
  ${GUI_suit}
  ${GUI_SUITApp}
  ${GUI_CAM}
  ${GUI_std}
  ${GEOM_GEOMBase}
  ${GEOM_GEOMFiltersSelection}
  ${GEOM_GEOMObject}
  SMESHFiltersSelection
  SMDS
  SMESHControls
  SMESHObject
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  SMESHGUI.h
  SMESHGUI_PatternWidget.h
  SMESHGUI_MeshPatternDlg.h
  SMESHGUI_CreatePatternDlg.h
  SMESHGUI_NodesDlg.h
  SMESHGUI_SpinBox.h
  SMESHGUI_TransparencyDlg.h
  SMESHGUI_ClippingDlg.h
  SMESHGUI_GroupDlg.h
  SMESHGUI_RemoveNodesDlg.h
  SMESHGUI_RemoveElementsDlg.h
  SMESHGUI_MeshInfo.h
  SMESHGUI_Measurements.h
  SMESHGUI_Preferences_ScalarBarDlg.h
  SMESHGUI_AddMeshElementDlg.h
  SMESHGUI_GroupOnShapeDlg.h
  SMESHGUI_FileInfoDlg.h
  SMESHGUI_FilterDlg.h
  SMESHGUI_FilterLibraryDlg.h
  SMESHGUI_SingleEditDlg.h
  SMESHGUI_MultiEditDlg.h
  SMESHGUI_DeleteGroupDlg.h
  SMESHGUI_GroupOpDlg.h
  SMESHGUI_SmoothingDlg.h
  SMESHGUI_RenumberingDlg.h
  SMESHGUI_ExtrusionDlg.h
  SMESHGUI_ExtrusionAlongPathDlg.h
  SMESHGUI_RevolutionDlg.h
  SMESHGUI_RotationDlg.h
  SMESHGUI_TranslationDlg.h
  SMESHGUI_ScaleDlg.h
  SMESHGUI_SymmetryDlg.h
  SMESHGUI_SewingDlg.h
  SMESHGUI_DuplicateNodesDlg.h
  SMESHGUI_MergeDlg.h
  SMESHGUI_CreatePolyhedralVolumeDlg.h
  SMESHGUI_Operation.h
  SMESHGUI_SelectionOp.h
  SMESHGUI_Dialog.h
  SMESHGUI_MeshDlg.h
  SMESHGUI_MeshOp.h
  SMESHGUI_Hypotheses.h
  SMESHGUI_ShapeByMeshDlg.h
  SMESHGUI_AddQuadraticElementDlg.h
  SMESHGUI_ConvToQuadDlg.h
  SMESHGUI_ConvToQuadOp.h
  SMESHGUI_BuildCompoundDlg.h
  SMESHGUI_ComputeDlg.h
  SMESHGUI_MakeNodeAtPointDlg.h
  SMESHGUI_MeshInfosBox.h
  SMESHGUI_Make2DFrom3DOp.h
  SMESHGUI_FindElemByPointDlg.h
  SMESHGUI_MeshOrderDlg.h
  SMESHGUI_CopyMeshDlg.h
  SMESHGUI_MeshOrderOp.h
  SMESHGUI_PreviewDlg.h
  SMESHGUI_ReorientFacesDlg.h
  SMESHGUI_PropertiesDlg.h
  SMESHGUI_Add0DElemsOnAllNodesDlg.h
  SMESHGUI_FieldSelectorWdg.h
  SMESHGUI_DisplayEntitiesDlg.h
  SMESHGUI_SplitBiQuad.h
  SMESHGUI_PreVisualObj.h
  SMESHGUI_IdPreview.h
)

# header files / no moc processing
SET(_other_HEADERS
  SMESHGUI_Operations.h
  SMESHGUI_Utils.h
  SMESHGUI_HypothesesUtils.h
  SMESHGUI_Selection.h
  SMESHGUI_VTKUtils.h
  SMESHGUI_Displayer.h
  SMESHGUI_GEOMGenUtils.h
  SMESHGUI_MeshUtils.h
  SMESHGUI_FilterUtils.h
  SMESHGUI_PatternUtils.h
  SMESHGUI_GroupUtils.h
  SMESHGUI_XmlHandler.h
  SMESHGUI_Filter.h
  SMESHGUI_MeshEditPreview.h
  SMESHGUI_IdValidator.h
  SMESHGUI_FileValidator.h
  SMESH_SMESHGUI.hxx
)

# header files / to install
SET(SMESH_HEADERS ${_moc_HEADERS} ${_other_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  SMESHGUI.cxx
  SMESHGUI_PatternWidget.cxx
  SMESHGUI_MeshPatternDlg.cxx
  SMESHGUI_CreatePatternDlg.cxx
  SMESHGUI_NodesDlg.cxx
  SMESHGUI_SpinBox.cxx
  SMESHGUI_TransparencyDlg.cxx
  SMESHGUI_ClippingDlg.cxx
  SMESHGUI_GroupDlg.cxx
  SMESHGUI_RemoveNodesDlg.cxx
  SMESHGUI_RemoveElementsDlg.cxx
  SMESHGUI_MeshInfo.cxx
  SMESHGUI_Measurements.cxx
  SMESHGUI_Preferences_ScalarBarDlg.cxx
  SMESHGUI_AddMeshElementDlg.cxx
  SMESHGUI_FilterDlg.cxx
  SMESHGUI_FilterLibraryDlg.cxx
  SMESHGUI_SingleEditDlg.cxx
  SMESHGUI_MultiEditDlg.cxx
  SMESHGUI_DeleteGroupDlg.cxx
  SMESHGUI_GroupOpDlg.cxx
  SMESHGUI_SmoothingDlg.cxx
  SMESHGUI_RenumberingDlg.cxx
  SMESHGUI_ExtrusionDlg.cxx
  SMESHGUI_ExtrusionAlongPathDlg.cxx
  SMESHGUI_RevolutionDlg.cxx
  SMESHGUI_RotationDlg.cxx
  SMESHGUI_TranslationDlg.cxx
  SMESHGUI_ScaleDlg.cxx
  SMESHGUI_SymmetryDlg.cxx
  SMESHGUI_SewingDlg.cxx
  SMESHGUI_DuplicateNodesDlg.cxx
  SMESHGUI_MergeDlg.cxx
  SMESHGUI_CreatePolyhedralVolumeDlg.cxx
  SMESHGUI_Operation.cxx
  SMESHGUI_SelectionOp.cxx
  SMESHGUI_Dialog.cxx
  SMESHGUI_MeshDlg.cxx
  SMESHGUI_MeshOp.cxx
  SMESHGUI_Hypotheses.cxx
  SMESHGUI_ShapeByMeshDlg.cxx
  SMESHGUI_AddQuadraticElementDlg.cxx
  SMESHGUI_ConvToQuadDlg.cxx
  SMESHGUI_ConvToQuadOp.cxx
  SMESHGUI_BuildCompoundDlg.cxx
  SMESHGUI_ComputeDlg.cxx
  SMESHGUI_MakeNodeAtPointDlg.cxx
  SMESHGUI_GroupOnShapeDlg.cxx
  SMESHGUI_FileInfoDlg.cxx
  SMESHGUI_MeshInfosBox.cxx
  SMESHGUI_Make2DFrom3DOp.cxx
  SMESHGUI_FindElemByPointDlg.cxx
  SMESHGUI_MeshOrderDlg.cxx
  SMESHGUI_CopyMeshDlg.cxx
  SMESHGUI_MeshOrderOp.cxx
  SMESHGUI_PreviewDlg.cxx
  SMESHGUI_ReorientFacesDlg.cxx
  SMESHGUI_PropertiesDlg.cxx
  SMESHGUI_Add0DElemsOnAllNodesDlg.cxx
  SMESHGUI_Utils.cxx
  SMESHGUI_HypothesesUtils.cxx
  SMESHGUI_Selection.cxx
  SMESHGUI_VTKUtils.cxx
  SMESHGUI_Displayer.cxx
  SMESHGUI_GEOMGenUtils.cxx
  SMESHGUI_MeshUtils.cxx
  SMESHGUI_FilterUtils.cxx
  SMESHGUI_PatternUtils.cxx
  SMESHGUI_GroupUtils.cxx
  SMESHGUI_XmlHandler.cxx
  SMESHGUI_Filter.cxx
  SMESHGUI_MeshEditPreview.cxx
  SMESHGUI_FileValidator.cxx
  SMESHGUI_FieldSelectorWdg.cxx
  SMESHGUI_DisplayEntitiesDlg.cxx
  SMESHGUI_SplitBiQuad.cxx
  SMESHGUI_PreVisualObj.cxx
  SMESHGUI_IdPreview.cxx
)

# sources / to compile
SET(SMESH_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  SMESH_images.ts
  SMESH_msg_en.ts
  SMESH_msg_fr.ts
  SMESH_msg_ja.ts
) 

# --- rules ---

ADD_LIBRARY(SMESH ${SMESH_SOURCES})
TARGET_LINK_LIBRARIES(SMESH ${_link_LIBRARIES} )
INSTALL(TARGETS SMESH EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SMESH_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})

QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_SMESH_INSTALL_RES_DATA}")
