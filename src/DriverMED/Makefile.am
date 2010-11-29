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

#  SMESH DriverMED : driver to read and write 'med' files
#  File   : Makefile.in
#  Author : Marc Tajchman (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#  $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	DriverMED_R_SMDS_Mesh.h \
	DriverMED_R_SMESHDS_Mesh.h \
	DriverMED_R_SMESHDS_Document.h \
	DriverMED_W_SMDS_Mesh.h \
	DriverMED_W_SMESHDS_Mesh.h \
	DriverMED_W_SMESHDS_Document.h \
	DriverMED_Family.h \
	SMESH_DriverMED.hxx

# Libraries targets
lib_LTLIBRARIES = libMeshDriverMED.la
dist_libMeshDriverMED_la_SOURCES = \
	DriverMED_R_SMDS_Mesh.cxx \
	DriverMED_R_SMESHDS_Mesh.cxx \
	DriverMED_R_SMESHDS_Document.cxx \
	DriverMED_W_SMDS_Mesh.cxx \
	DriverMED_W_SMESHDS_Document.cxx \
	DriverMED_W_SMESHDS_Mesh.cxx \
	DriverMED_Family.cxx

# Executables targets
bin_PROGRAMS = MED_Test

dist_MED_Test_SOURCES = \
	MED_Test.cxx


# additionnal information to compil and link file
libMeshDriverMED_la_CPPFLAGS = \
	$(MED_CXXFLAGS) \
	@HDF5_INCLUDES@ \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS

libMeshDriverMED_la_LDFLAGS  = \
	$(BOOST_LIBS) \
	../Driver/libMeshDriver.la \
	$(MED_LDFLAGS) -lMEDWrapper -lMEDWrapperBase -lMEDWrapper_V2_1 -lMEDWrapper_V2_2


MED_Test_CPPFLAGS = \
	$(libMeshDriverMED_la_CPPFLAGS)

MED_Test_LDADD = \
	libMeshDriverMED.la \
	../Driver/libMeshDriver.la \
	../SMDS/libSMDS.la \
	../SMESHDS/libSMESHDS.la \
	$(KERNEL_LDFLAGS) \
	-lOpUtil \
	-lSALOMELocalTrace \
	-lSALOMEBasics \
	$(MED_LDFLAGS) \
	-lMEDWrapper \
	-lMEDWrapperBase \
	-lMEDWrapper_V2_1 \
	-lmed_V2_1 \
	-lMEDWrapper_V2_2

