# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

# SMESH SMESHGUI : GUI for SMESH component
# File   : Makefile.am
# Author : Alexander BORODIN, Open CASCADE S.A.S.
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESHGUI.h \
	SMESHGUI_Utils.h \
	SMESHGUI_Hypotheses.h \
	SMESHGUI_HypothesesUtils.h \
	SMESHGUI_SpinBox.h \
	SMESHGUI_Selection.h \
	SMESHGUI_VTKUtils.h \
	SMESHGUI_Displayer.h \
	SMESHGUI_GEOMGenUtils.h \
	SMESHGUI_MeshUtils.h \
	SMESHGUI_FilterUtils.h \
	SMESHGUI_PatternUtils.h \
	SMESHGUI_GroupUtils.h \
	SMESHGUI_PatternWidget.h \
	SMESHGUI_CreatePatternDlg.h \
	SMESHGUI_MeshPatternDlg.h \
	SMESHGUI_NodesDlg.h \
	SMESHGUI_TransparencyDlg.h \
	SMESHGUI_ClippingDlg.h \
	SMESHGUI_GroupDlg.h \
	SMESHGUI_RemoveNodesDlg.h \
	SMESHGUI_RemoveElementsDlg.h \
	SMESHGUI_MeshInfo.h \
	SMESHGUI_Measurements.h \
	SMESHGUI_Preferences_ColorDlg.h \
	SMESHGUI_Preferences_ScalarBarDlg.h \
	SMESHGUI_AddMeshElementDlg.h \
	SMESHGUI_XmlHandler.h \
	SMESHGUI_Filter.h \
	SMESHGUI_FilterDlg.h \
	SMESHGUI_FilterLibraryDlg.h \
	SMESHGUI_SingleEditDlg.h \
	SMESHGUI_MultiEditDlg.h \
	SMESHGUI_DeleteGroupDlg.h \
	SMESHGUI_GroupOpDlg.h \
	SMESHGUI_SmoothingDlg.h \
	SMESHGUI_RenumberingDlg.h \
	SMESHGUI_ExtrusionDlg.h \
	SMESHGUI_ExtrusionAlongPathDlg.h \
	SMESHGUI_RevolutionDlg.h \
	SMESHGUI_RotationDlg.h \
	SMESHGUI_TranslationDlg.h \
	SMESHGUI_ScaleDlg.h \
	SMESHGUI_SymmetryDlg.h \
	SMESHGUI_SewingDlg.h \
	SMESHGUI_DuplicateNodesDlg.h \
	SMESHGUI_MergeDlg.h \
	SMESHGUI_MeshUtils.h \
	SMESHGUI_CreatePolyhedralVolumeDlg.h \
	SMESHGUI_Operation.h \
	SMESHGUI_SelectionOp.h \
	SMESHGUI_Dialog.h \
	SMESHGUI_MeshDlg.h \
	SMESHGUI_MeshOp.h \
	SMESHGUI_ShapeByMeshDlg.h \
	SMESHGUI_AddQuadraticElementDlg.h \
	SMESHGUI_ConvToQuadDlg.h \
	SMESHGUI_ConvToQuadOp.h \
	SMESHGUI_BuildCompoundDlg.h \
	SMESHGUI_ComputeDlg.h \
	SMESHGUI_MakeNodeAtPointDlg.h \
	SMESHGUI_MeshEditPreview.h \
	SMESHGUI_IdValidator.h \
	SMESHGUI_MeshInfosBox.h \
	SMESHGUI_Make2DFrom3DOp.h \
	SMESHGUI_FindElemByPointDlg.h \
	SMESHGUI_MeshOrderDlg.h \
	SMESHGUI_MeshOrderOp.h \
	SMESHGUI_FileValidator.h \
	SMESHGUI_CopyMeshDlg.h \
	SMESHGUI_PreviewDlg.h \
	SMESH_SMESHGUI.hxx

# Libraries targets
lib_LTLIBRARIES = libSMESH.la
dist_libSMESH_la_SOURCES = \
	SMESHGUI.cxx \
	SMESHGUI_PatternWidget.cxx \
	SMESHGUI_CreatePatternDlg.cxx \
	SMESHGUI_MeshPatternDlg.cxx \
	SMESHGUI_SpinBox.cxx \
	SMESHGUI_NodesDlg.cxx \
	SMESHGUI_TransparencyDlg.cxx \
	SMESHGUI_ClippingDlg.cxx \
	SMESHGUI_GroupDlg.cxx \
	SMESHGUI_RemoveNodesDlg.cxx \
	SMESHGUI_RemoveElementsDlg.cxx \
	SMESHGUI_MeshInfo.cxx \
	SMESHGUI_Measurements.cxx \
	SMESHGUI_Preferences_ColorDlg.cxx \
	SMESHGUI_Preferences_ScalarBarDlg.cxx \
	SMESHGUI_AddMeshElementDlg.cxx \
	SMESHGUI_XmlHandler.cxx \
	SMESHGUI_Filter.cxx \
	SMESHGUI_FilterDlg.cxx \
	SMESHGUI_FilterLibraryDlg.cxx \
	SMESHGUI_SingleEditDlg.cxx \
	SMESHGUI_MultiEditDlg.cxx \
	SMESHGUI_DeleteGroupDlg.cxx \
	SMESHGUI_GroupOpDlg.cxx \
	SMESHGUI_SmoothingDlg.cxx \
	SMESHGUI_RenumberingDlg.cxx \
	SMESHGUI_ExtrusionDlg.cxx \
	SMESHGUI_ExtrusionAlongPathDlg.cxx \
	SMESHGUI_RevolutionDlg.cxx \
	SMESHGUI_RotationDlg.cxx \
	SMESHGUI_TranslationDlg.cxx \
	SMESHGUI_ScaleDlg.cxx \
	SMESHGUI_SymmetryDlg.cxx \
	SMESHGUI_SewingDlg.cxx \
	SMESHGUI_DuplicateNodesDlg.cxx \
	SMESHGUI_MergeDlg.cxx \
	SMESHGUI_Utils.cxx \
	SMESHGUI_GEOMGenUtils.cxx \
	SMESHGUI_MeshUtils.cxx \
	SMESHGUI_GroupUtils.cxx \
	SMESHGUI_FilterUtils.cxx \
	SMESHGUI_PatternUtils.cxx \
	SMESHGUI_HypothesesUtils.cxx \
	SMESHGUI_VTKUtils.cxx \
	SMESHGUI_Selection.cxx \
	SMESHGUI_CreatePolyhedralVolumeDlg.cxx \
	SMESHGUI_Operation.cxx \
	SMESHGUI_SelectionOp.cxx \
	SMESHGUI_Dialog.cxx \
	SMESHGUI_MeshDlg.cxx \
	SMESHGUI_MeshOp.cxx \
	SMESHGUI_Displayer.cxx \
	SMESHGUI_Hypotheses.cxx \
	SMESHGUI_ShapeByMeshDlg.cxx \
	SMESHGUI_AddQuadraticElementDlg.cxx \
	SMESHGUI_ConvToQuadDlg.cxx \
	SMESHGUI_ConvToQuadOp.cxx \
	SMESHGUI_BuildCompoundDlg.cxx \
	SMESHGUI_ComputeDlg.cxx \
	SMESHGUI_MakeNodeAtPointDlg.cxx \
	SMESHGUI_MeshEditPreview.cxx \
	SMESHGUI_GroupOnShapeDlg.cxx \
	SMESHGUI_FileInfoDlg.cxx \
	SMESHGUI_MeshInfosBox.cxx \
	SMESHGUI_Make2DFrom3DOp.cxx \
	SMESHGUI_FindElemByPointDlg.cxx \
	SMESHGUI_MeshOrderDlg.cxx \
	SMESHGUI_MeshOrderOp.cxx \
	SMESHGUI_CopyMeshDlg.cxx \
	SMESHGUI_FileValidator.cxx \
	SMESHGUI_PreviewDlg.cxx

MOC_FILES = \
	SMESHGUI_moc.cxx \
	SMESHGUI_PatternWidget_moc.cxx \
	SMESHGUI_MeshPatternDlg_moc.cxx \
	SMESHGUI_CreatePatternDlg_moc.cxx \
	SMESHGUI_NodesDlg_moc.cxx \
	SMESHGUI_SpinBox_moc.cxx \
	SMESHGUI_TransparencyDlg_moc.cxx \
	SMESHGUI_ClippingDlg_moc.cxx \
	SMESHGUI_GroupDlg_moc.cxx \
	SMESHGUI_RemoveNodesDlg_moc.cxx \
	SMESHGUI_RemoveElementsDlg_moc.cxx \
	SMESHGUI_MeshInfo_moc.cxx \
	SMESHGUI_Measurements_moc.cxx \
	SMESHGUI_Preferences_ColorDlg_moc.cxx \
	SMESHGUI_Preferences_ScalarBarDlg_moc.cxx \
	SMESHGUI_AddMeshElementDlg_moc.cxx \
	SMESHGUI_FilterDlg_moc.cxx \
	SMESHGUI_FilterLibraryDlg_moc.cxx \
	SMESHGUI_SingleEditDlg_moc.cxx \
	SMESHGUI_MultiEditDlg_moc.cxx \
	SMESHGUI_DeleteGroupDlg_moc.cxx \
	SMESHGUI_GroupOpDlg_moc.cxx \
	SMESHGUI_SmoothingDlg_moc.cxx \
	SMESHGUI_RenumberingDlg_moc.cxx \
	SMESHGUI_ExtrusionDlg_moc.cxx \
	SMESHGUI_ExtrusionAlongPathDlg_moc.cxx \
	SMESHGUI_RevolutionDlg_moc.cxx \
	SMESHGUI_RotationDlg_moc.cxx \
	SMESHGUI_TranslationDlg_moc.cxx \
	SMESHGUI_ScaleDlg_moc.cxx \
	SMESHGUI_SymmetryDlg_moc.cxx \
	SMESHGUI_SewingDlg_moc.cxx \
	SMESHGUI_DuplicateNodesDlg_moc.cxx \
	SMESHGUI_MergeDlg_moc.cxx \
	SMESHGUI_CreatePolyhedralVolumeDlg_moc.cxx \
	SMESHGUI_Operation_moc.cxx \
	SMESHGUI_SelectionOp_moc.cxx \
	SMESHGUI_Dialog_moc.cxx \
	SMESHGUI_MeshDlg_moc.cxx \
	SMESHGUI_MeshOp_moc.cxx \
	SMESHGUI_Hypotheses_moc.cxx \
	SMESHGUI_ShapeByMeshDlg_moc.cxx \
	SMESHGUI_AddQuadraticElementDlg_moc.cxx \
	SMESHGUI_ConvToQuadDlg_moc.cxx \
	SMESHGUI_ConvToQuadOp_moc.cxx \
	SMESHGUI_BuildCompoundDlg_moc.cxx \
	SMESHGUI_ComputeDlg_moc.cxx \
	SMESHGUI_MakeNodeAtPointDlg_moc.cxx \
	SMESHGUI_GroupOnShapeDlg_moc.cxx \
	SMESHGUI_FileInfoDlg_moc.cxx \
	SMESHGUI_MeshInfosBox_moc.cxx \
	SMESHGUI_Make2DFrom3DOp_moc.cxx \
	SMESHGUI_FindElemByPointDlg_moc.cxx \
	SMESHGUI_MeshOrderDlg_moc.cxx \
	SMESHGUI_CopyMeshDlg_moc.cxx \
	SMESHGUI_MeshOrderOp_moc.cxx \
	SMESHGUI_PreviewDlg_moc.cxx

nodist_libSMESH_la_SOURCES= \
	$(MOC_FILES)

# additionnal information to compil and link file
libSMESH_la_CPPFLAGS = \
	$(QT_INCLUDES) \
	$(VTK_INCLUDES) \
	$(OGL_INCLUDES) \
	$(CAS_CPPFLAGS) \
	$(PYTHON_INCLUDES) \
	$(KERNEL_CXXFLAGS)  \
	$(GUI_CXXFLAGS) \
	$(QWT_INCLUDES) \
	$(GEOM_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	$(MED3_INCLUDES) \
	$(HDF5_INCLUDES) \
	-I$(srcdir)/../OBJECT \
	-I$(srcdir)/../SMESHFiltersSelection \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../SMESH \
	-I$(srcdir)/../SMESHUtils \
	-I$(srcdir)/../SMESH_I \
	-I$(srcdir)/../Controls \
	-I$(srcdir)/../SMESHClient \
	-I$(top_builddir) \
	-I$(top_builddir)/idl

libSMESH_la_LDFLAGS  = \
	../SMESHFiltersSelection/libSMESHFiltersSelection.la \
	../SMDS/libSMDS.la \
	../Controls/libSMESHControls.la \
	../OBJECT/libSMESHObject.la \
	$(CAS_LDFLAGS) -lTKV3d \
	$(GUI_LDFLAGS) -lSalomePrs -lsuit -lSUITApp -lCAM -lstd \
	$(GEOM_LDFLAGS) -lGEOMBase -lGEOMFiltersSelection -lGEOMObject \
	$(PYTHON_LIBS)

# resources files
nodist_salomeres_DATA= \
	SMESH_images.qm \
	SMESH_msg_en.qm \
	SMESH_msg_fr.qm
