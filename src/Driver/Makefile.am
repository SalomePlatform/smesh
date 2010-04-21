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

#  SMESH Driver : implementaion of driver for reading and writing	
#  File   : Makefile.in
#  Author : Marc Tajchman (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#  $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	Driver_Document.h \
	Driver_Mesh.h \
	Driver_SMDS_Mesh.h \
	Driver_SMESHDS_Mesh.h

# Libraries targets
lib_LTLIBRARIES = libMeshDriver.la

dist_libMeshDriver_la_SOURCES = \
	Driver_Document.cxx \
	Driver_Mesh.cxx \
	Driver_SMDS_Mesh.cxx \
	Driver_SMESHDS_Mesh.cxx

# additionnal information to compil and link file
libMeshDriver_la_CPPFLAGS = \
	$(CAS_CPPFLAGS) \
	$(KERNEL_CXXFLAGS) \
	-I$(srcdir)/../SMESHDS	

libMeshDriver_la_LDFLAGS  = \
	../SMESHDS/libSMESHDS.la \
	$(KERNEL_LDFLAGS) -lOpUtil \
	$(CAS_LDPATH) -lTKernel -lTKTopAlgo
