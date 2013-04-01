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

#  SMESH DriverUNV : driver to read and write 'unv' files
#  File   : Makefile.in
#  Author : Marc Tajchman (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#  $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	DriverUNV_R_SMDS_Mesh.h \
	DriverUNV_W_SMDS_Mesh.h \
	SMESH_DriverUNV.hxx

# not-exported (internal) files
EXTRA_DIST += \
	UNV164_Structure.hxx \
	UNV2411_Structure.hxx \
	UNV2412_Structure.hxx \
	UNV2417_Structure.hxx \
	UNV2420_Structure.hxx \
	UNV_Utilities.hxx

# Libraries targets
lib_LTLIBRARIES = libMeshDriverUNV.la
dist_libMeshDriverUNV_la_SOURCES = \
	DriverUNV_R_SMDS_Mesh.cxx \
	DriverUNV_W_SMDS_Mesh.cxx \
	UNV_Utilities.cxx \
	UNV164_Structure.cxx \
	UNV2411_Structure.cxx \
	UNV2412_Structure.cxx \
	UNV2420_Structure.cxx \
	UNV2417_Structure.cxx

# Executables targets
bin_PROGRAMS = UNV_Test

UNV_Test_SOURCES = \
	UNV_Test.cxx

# additionnal information to compil and link file
libMeshDriverUNV_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHUtils \
	-I$(srcdir)/../SMESHDS

libMeshDriverUNV_la_LDFLAGS  = \
	../Driver/libMeshDriver.la \
	$(KERNEL_LDFLAGS) -lSALOMEBasics \
	$(CAS_KERNEL)

UNV_Test_CPPFLAGS = \
	$(libMeshDriverUNV_la_CPPFLAGS)

UNV_Test_LDADD = \
	libMeshDriverUNV.la \
	../Driver/libMeshDriver.la \
	../SMDS/libSMDS.la \
	$(KERNEL_LDFLAGS) -lOpUtil -lSALOMELocalTrace -lSALOMEBasics
