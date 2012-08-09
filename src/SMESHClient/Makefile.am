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

#  GEOM GEOMClient : tool to transfer BREP files from GEOM server to GEOM client
#  File   : Makefile.in
#  Author : Pavel TELKOV (OCC)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SHESM
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESH_Client.hxx

# Libraries targets
lib_LTLIBRARIES = libSMESHClient.la
dist_libSMESHClient_la_SOURCES = \
	SMESH_Client.cxx

# Executables targets
bin_PROGRAMS = SMESHClientBin
dist_SMESHClientBin_SOURCES = \
	SMESHClientBin.cxx

# additionnal information to compil and link file
libSMESHClient_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(GEOM_CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(CAS_CPPFLAGS) \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	-I$(srcdir)/../Controls \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../DriverDAT \
	-I$(srcdir)/../DriverMED \
	-I$(srcdir)/../DriverUNV \
	-I$(srcdir)/../DriverSTL \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../SMESH \
	-I$(top_builddir)/idl

libSMESHClient_la_LDFLAGS  = \
	../../idl/libSalomeIDLSMESH.la \
	../SMDS/libSMDS.la \
	../SMESH/libSMESHimpl.la \
	../SMESHDS/libSMESHDS.la \
	../Controls/libSMESHControls.la \
	$(KERNEL_LDFLAGS) -lSalomeLifeCycleCORBA -lSalomeNS -lOpUtil -lSALOMEBasics \
	-lSalomeIDLKernel -lSALOMELocalTrace \
	$(GEOM_LDFLAGS) -lNMTDS -lSalomeIDLGEOM \
	$(MED_LDFLAGS) -lMEDWrapper_V2_2 -lMEDWrapper_V2_1 -lSalomeIDLMED -lMEDWrapper \
	$(CAS_KERNEL)

SMESHClientBin_CPPFLAGS = \
	$(libSMESHClient_la_CPPFLAGS)

SMESHClientBin_LDADD = \
	$(libSMESHClient_la_LDFLAGS) @CORBA_LIBS@


