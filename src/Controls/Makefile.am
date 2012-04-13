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
#  Author : Patrick GOLDBRONN (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESH_ControlsDef.hxx

# Libraries targets
lib_LTLIBRARIES = libSMESHControls.la

dist_libSMESHControls_la_SOURCES = \
	SMESH_Controls.cxx

# Executables targets
bin_PROGRAMS = SMESHControls
dist_SMESHControls_SOURCES = \
	SMESHControls.cxx

# additionnal information to compil and link file
libSMESHControls_la_CPPFLAGS = \
	$(CAS_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	$(KERNEL_CXXFLAGS) \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../SMESHUtils

libSMESHControls_la_LDFLAGS = \
	../SMDS/libSMDS.la \
	../SMESHDS/libSMESHDS.la \
	../SMESHUtils/libSMESHUtils.la \
	$(CAS_LDPATH) -lTKernel -lTKBRep -lTKG3d -lTKTopAlgo -lTKGeomBase -lTKGeomAlgo \
	$(VTK_LIBS)

SMESHControls_CPPFLAGS = \
	$(libSMESHControls_la_CPPFLAGS)


SMESHControls_LDADD = \
	../SMDS/libSMDS.la \
	../SMESHDS/libSMESHDS.la \
	libSMESHControls.la \
	$(KERNEL_LDFLAGS) -lOpUtil -lSALOMELocalTrace -lSALOMEBasics \
	$(GEOM_LDFLAGS) -lGEOMAlgo \
	$(CAS_LDPATH) -lTKernel -lTKBRep -lTKG3d
