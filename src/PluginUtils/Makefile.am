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

# ---
# File   : Makefile.am
# Author : Nicolas GEIMER (OCC)
# ---

include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	GeomSelectionTools.h

# Libraries targets
lib_LTLIBRARIES = libGeomSelectionTools.la

dist_libGeomSelectionTools_la_SOURCES =		\
	GeomSelectionTools.h            	\
	GeomSelectionTools.cxx

# additionnal information to compil and link file
libGeomSelectionTools_la_CPPFLAGS = 	\
	$(QT_INCLUDES)			\
	$(CAS_CPPFLAGS)			\
	$(PYTHON_INCLUDES)		\
	$(KERNEL_CXXFLAGS)		\
	$(GUI_CXXFLAGS)			\
	$(GEOM_CXXFLAGS)		\
	$(MED_CXXFLAGS)   		\
	$(BOOST_CPPFLAGS)		\
	$(CORBA_CXXFLAGS)		\
	$(CORBA_INCLUDES)		\
	-I$(srcdir)/../SMESHGUI		\
	-I$(top_builddir)/idl

libGeomSelectionTools_la_LDFLAGS =		\
	../../idl/libSalomeIDLSMESH.la \
	../SMESHGUI/libSMESH.la \
	$(QT_LIBS) \
	$(CORBA_LIBS) \
	$(KERNEL_LDFLAGS) -lSalomeIDLKernel -lSALOMELocalTrace -lSalomeLifeCycleCORBA \
	$(GUI_LDFLAGS) -lSalomeObject -lsuit -lLightApp -lSalomeApp \
	$(GEOM_LDFLAGS) -lSalomeIDLGEOM -lGEOMClient \
	$(CAS_KERNEL) -lTKBRep -lTKG3d
