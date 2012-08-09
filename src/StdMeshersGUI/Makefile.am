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
#  File   : Makefile.in
#  Author : Julia DOROVSKIKH
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH

include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	StdMeshersGUI_StdHypothesisCreator.h \
	SMESH_StdMeshersGUI.hxx \
	StdMeshersGUI_DistrPreview.h \
	StdMeshersGUI_DistrTable.h \
	StdMeshersGUI_NbSegmentsCreator.h \
	StdMeshersGUI_ObjectReferenceParamWdg.h \
	StdMeshersGUI_QuadrangleParamWdg.h \
	StdMeshersGUI_LayerDistributionParamWdg.h \
	StdMeshersGUI_FixedPointsParamWdg.h \
	StdMeshersGUI_SubShapeSelectorWdg.h \
	StdMeshersGUI_CartesianParamCreator.h

# Libraries targets
lib_LTLIBRARIES = libStdMeshersGUI.la

dist_libStdMeshersGUI_la_SOURCES = \
	StdMeshersGUI.cxx \
	StdMeshersGUI_StdHypothesisCreator.cxx \
	StdMeshersGUI_DistrPreview.cxx \
	StdMeshersGUI_DistrTable.cxx \
	StdMeshersGUI_NbSegmentsCreator.cxx \
	StdMeshersGUI_ObjectReferenceParamWdg.cxx \
	StdMeshersGUI_QuadrangleParamWdg.cxx \
	StdMeshersGUI_LayerDistributionParamWdg.cxx \
	StdMeshersGUI_FixedPointsParamWdg.cxx \
	StdMeshersGUI_SubShapeSelectorWdg.cxx \
	StdMeshersGUI_CartesianParamCreator.cxx

MOC_FILES = \
	StdMeshersGUI_StdHypothesisCreator_moc.cxx \
	StdMeshersGUI_DistrPreview_moc.cxx \
	StdMeshersGUI_DistrTable_moc.cxx \
	StdMeshersGUI_NbSegmentsCreator_moc.cxx \
	StdMeshersGUI_ObjectReferenceParamWdg_moc.cxx \
	StdMeshersGUI_QuadrangleParamWdg_moc.cxx \
	StdMeshersGUI_LayerDistributionParamWdg_moc.cxx \
	StdMeshersGUI_FixedPointsParamWdg_moc.cxx \
	StdMeshersGUI_SubShapeSelectorWdg_moc.cxx \
	StdMeshersGUI_CartesianParamCreator_moc.cxx

nodist_libStdMeshersGUI_la_SOURCES= \
	$(MOC_FILES)

# additionnal information to compil and link file
libStdMeshersGUI_la_CPPFLAGS = \
	$(QT_INCLUDES) \
	$(VTK_INCLUDES) \
	$(CAS_CPPFLAGS) \
	$(PYTHON_INCLUDES) \
	$(KERNEL_CXXFLAGS) \
	$(GUI_CXXFLAGS) \
	$(GEOM_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(QWT_INCLUDES) \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	-I$(srcdir)/../SMESH \
	-I$(srcdir)/../SMESHUtils \
	-I$(srcdir)/../SMESH_I \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHGUI \
	-I$(srcdir)/../Controls \
	-I$(srcdir)/../StdMeshers \
	-I$(srcdir)/../OBJECT \
	-I$(srcdir)/../SMESHFiltersSelection \
	-I$(top_builddir)/idl

libStdMeshersGUI_la_LDFLAGS  = \
	../../idl/libSalomeIDLSMESH.la \
	../SMESHFiltersSelection/libSMESHFiltersSelection.la \
	../SMESHGUI/libSMESH.la \
	../OBJECT/libSMESHObject.la \
	$(GUI_LDFLAGS) -lSalomeApp \
	$(CAS_LDPATH) -lTKernel -lTKBO -lTKAdvTools \
	$(QWT_LIBS)

# resources files
nodist_salomeres_DATA= \
	StdMeshers_images.qm \
	StdMeshers_msg_en.qm \
	StdMeshers_msg_fr.qm
