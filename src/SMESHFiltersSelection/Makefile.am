# Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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

#  SMESH SMESHFiltersSelection : filter selector for viewer
#  File   : Makefile.in
#  Author : Patrick GOLDBRONN (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#  $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESH_Type.h \
	SMESH_TypeFilter.hxx \
	SMESH_NumberFilter.hxx \
	SMESH_LogicalFilter.hxx

# Libraries targets

lib_LTLIBRARIES = libSMESHFiltersSelection.la 

dist_libSMESHFiltersSelection_la_SOURCES = \
	SMESH_TypeFilter.cxx \
	SMESH_NumberFilter.cxx \
	SMESH_LogicalFilter.cxx

# additionnal information to compil and link file
libSMESHFiltersSelection_la_CPPFLAGS = \
	$(CAS_CPPFLAGS) \
	$(QT_INCLUDES) \
	$(PYTHON_INCLUDES) \
	$(VTK_INCLUDES) \
	$(KERNEL_CXXFLAGS) \
	$(GUI_CXXFLAGS) \
	$(GEOM_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	-I$(top_builddir)/idl

libSMESHFiltersSelection_la_LDFLAGS  = \
	../../idl/libSalomeIDLSMESH.la \
	$(KERNEL_LDFLAGS) -lSalomeDSClient -lSalomeDS \
	$(GUI_LDFLAGS) -lSalomeApp -lsuit -lLightApp \
	$(GEOM_LDFLAGS) -lGEOM -lGEOMClient \
	$(CAS_LDPATH) -lTKernel -lTKMath -lTKBRep \
	$(QT_MT_LIBS)
