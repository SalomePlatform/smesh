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

#  SMESH SMESHDS : management of mesh data and SMESH document
#  File   : Makefile.in
#  Author : Patrick GOLDBRONN (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESHDS_Document.hxx \
	SMESHDS_Hypothesis.hxx \
	SMESHDS_Mesh.hxx \
	SMESHDS_Script.hxx \
	SMESHDS_Command.hxx \
	SMESHDS_CommandType.hxx \
	SMESHDS_SubMesh.hxx \
	SMESHDS_GroupBase.hxx \
	SMESHDS_Group.hxx \
	SMESHDS_GroupOnGeom.hxx \
	SMESHDS_GroupOnFilter.hxx \
	SMESH_SMESHDS.hxx \
	SMESHDS_DataMapOfShape.hxx \
	SMESH_Controls.hxx

# Libraries targets
lib_LTLIBRARIES = libSMESHDS.la

dist_libSMESHDS_la_SOURCES = \
	SMESHDS_Document.cxx \
	SMESHDS_Hypothesis.cxx \
	SMESHDS_Script.cxx \
	SMESHDS_Command.cxx \
	SMESHDS_SubMesh.cxx \
	SMESHDS_Mesh.cxx \
	SMESHDS_GroupBase.cxx \
	SMESHDS_Group.cxx \
	SMESHDS_GroupOnGeom.cxx \
	SMESHDS_GroupOnFilter.cxx

# additionnal information to compil and link file
libSMESHDS_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(srcdir)/../SMDS

libSMESHDS_la_LDFLAGS  = \
	../SMDS/libSMDS.la \
	$(KERNEL_LDFLAGS) -lSALOMELocalTrace \
	$(CAS_KERNEL) -lTKBRep
