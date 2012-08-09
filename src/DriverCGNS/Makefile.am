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

include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	DriverCGNS_Read.hxx \
	DriverCGNS_Write.hxx \
	SMESH_DriverCGNS.hxx

# Libraries targets
lib_LTLIBRARIES = libMeshDriverCGNS.la
dist_libMeshDriverCGNS_la_SOURCES = \
	DriverCGNS_Read.cxx \
	DriverCGNS_Write.cxx

# additionnal information to compil and link file
libMeshDriverCGNS_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
	$(CGNS_INCLUDES) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../SMESHUtils \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS

libMeshDriverCGNS_la_LDFLAGS  = \
	$(BOOST_LIBS) \
	$(CGNS_LIBS) \
	../Driver/libMeshDriver.la \
	../SMESHUtils/libSMESHUtils.la
