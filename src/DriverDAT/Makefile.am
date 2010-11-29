#  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

#  SMESH DriverDAT : driver to read and write 'dat' files
#  File   : Makefile.in
#  Author : Marc Tajchman (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#  $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	DriverDAT_R_SMDS_Mesh.h \
	DriverDAT_R_SMESHDS_Mesh.h \
	DriverDAT_R_SMESHDS_Document.h \
	DriverDAT_W_SMDS_Mesh.h \
	DriverDAT_W_SMESHDS_Mesh.h \
	DriverDAT_W_SMESHDS_Document.h \
	SMESH_DriverDAT.hxx

# Libraries targets
lib_LTLIBRARIES = libMeshDriverDAT.la

dist_libMeshDriverDAT_la_SOURCES = \
	DriverDAT_R_SMDS_Mesh.cxx \
	DriverDAT_R_SMESHDS_Mesh.cxx \
	DriverDAT_R_SMESHDS_Document.cxx \
	DriverDAT_W_SMDS_Mesh.cxx \
	DriverDAT_W_SMESHDS_Mesh.cxx \
	DriverDAT_W_SMESHDS_Document.cxx

# Executables targets
bin_PROGRAMS = DAT_Test
dist_DAT_Test_SOURCES = \
	DAT_Test.cxx

# additionnal information to compil and link file
libMeshDriverDAT_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS

libMeshDriverDAT_la_LDFLAGS  = \
	../Driver/libMeshDriver.la \
	$(KERNEL_LDFLAGS) -lSALOMEBasics \
	$(CAS_KERNEL)

DAT_Test_CPPFLAGS = \
	$(libMeshDriverDAT_la_CPPFLAGS)

DAT_Test_LDADD = \
	libMeshDriverDAT.la \
	../Driver/libMeshDriver.la \
	../SMDS/libSMDS.la \
	$(KERNEL_LDFLAGS) -lOpUtil -lSALOMELocalTrace -lSALOMEBasics

