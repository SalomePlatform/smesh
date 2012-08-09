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
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESH_Block.hxx \
	SMESH_TypeDefs.hxx \
	SMESH_Octree.hxx \
	SMESH_OctreeNode.hxx \
	SMESH_Comment.hxx \
	SMESH_ComputeError.hxx \
	SMESH_File.hxx \
	SMESH_Utils.hxx

# Libraries targets

lib_LTLIBRARIES = libSMESHUtils.la

dist_libSMESHUtils_la_SOURCES = \
	SMESH_Block.cxx \
	SMESH_Octree.cxx \
	SMESH_OctreeNode.cxx \
	SMESH_File.cxx

# additionnal information to compile and link file
libSMESHUtils_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS

libSMESHUtils_la_LDFLAGS = \
	../SMESHDS/libSMESHDS.la \
	$(CAS_LDPATH) -lTKShHealing -lTKPrim -lTKG2d -lTKG3d -lTKGeomBase
