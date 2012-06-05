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

#  SMESH SMDS : implementaion of Salome mesh data structure
#  File   : Makefile.in
#  Author : Patrick GOLDBRONN (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
    chrono.hxx \
	ObjectPool.hxx \
	SMDS_TypeOfPosition.hxx \
	SMDSAbs_ElementType.hxx \
	SMDS_EdgePosition.hxx \
	SMDS_ElemIterator.hxx \
	SMDS_FacePosition.hxx \
	SMDS_Mesh.hxx \
	SMDS_Mesh0DElement.hxx \
	SMDS_LinearEdge.hxx \
	SMDS_MeshEdge.hxx \
	SMDS_MeshElement.hxx \
	SMDS_MeshElement.cxx \
	SMDS_MeshElementIDFactory.hxx \
	SMDS_MeshCell.hxx \
	SMDS_MeshFace.hxx \
	SMDS_MeshGroup.hxx \
	SMDS_MeshIDFactory.hxx \
	SMDS_MeshNode.hxx \
	SMDS_MeshNodeIDFactory.hxx \
	SMDS_MeshObject.hxx \
	SMDS_MeshVolume.hxx \
	SMDS_Position.hxx \
	SMDS_SpacePosition.hxx \
	SMDS_VertexPosition.hxx \
	SMDS_Iterator.hxx \
	SMDS_IteratorOfElements.hxx \
	SMDS_VolumeOfFaces.hxx \
	SMDS_VolumeOfNodes.hxx \
	SMDS_VtkEdge.hxx \
	SMDS_VtkFace.hxx \
	SMDS_VtkVolume.hxx \
	SMDS_VtkCellIterator.hxx \
	SMDS_PolyhedralVolumeOfNodes.hxx \
	SMDS_FaceOfEdges.hxx \
	SMDS_FaceOfNodes.hxx \
	SMDS_PolygonalFaceOfNodes.hxx \
	SMDS_VolumeTool.hxx \
	SMDS_QuadraticEdge.hxx \
	SMDS_QuadraticFaceOfNodes.hxx \
	SMDS_QuadraticVolumeOfNodes.hxx \
	SMDS_SetIterator.hxx \
	SMESH_SMDS.hxx \
	SMDS_MeshInfo.hxx \
	SMDS_UnstructuredGrid.hxx \
	SMDS_Downward.hxx \
	SMDS_StdIterator.hxx \
	SMDS_IteratorOnIterators.hxx


# Libraries targets

lib_LTLIBRARIES = libSMDS.la
dist_libSMDS_la_SOURCES = \
    chrono.cxx \
	SMDS_MeshObject.cxx \
	SMDS_MeshElement.cxx \
	SMDS_MeshCell.cxx \
	SMDS_Position.cxx \
	SMDS_EdgePosition.cxx \
	SMDS_FacePosition.cxx \
	SMDS_SpacePosition.cxx \
	SMDS_VertexPosition.cxx \
	SMDS_MeshNode.cxx \
	SMDS_Mesh0DElement.cxx \
	SMDS_LinearEdge.cxx \
	SMDS_MeshEdge.cxx \
	SMDS_MeshFace.cxx \
	SMDS_MeshVolume.cxx \
	SMDS_MeshNodeIDFactory.cxx \
	SMDS_MeshElementIDFactory.cxx \
	SMDS_MeshGroup.cxx \
	SMDS_MeshIDFactory.cxx \
	SMDS_Mesh.cxx \
	SMDS_IteratorOfElements.cxx \
	SMDS_VolumeOfFaces.cxx \
	SMDS_VolumeOfNodes.cxx \
	SMDS_VtkEdge.cxx \
	SMDS_VtkFace.cxx \
	SMDS_VtkVolume.cxx \
	SMDS_VtkCellIterator.cxx \
	SMDS_PolyhedralVolumeOfNodes.cxx \
	SMDS_FaceOfEdges.cxx \
	SMDS_FaceOfNodes.cxx \
	SMDS_PolygonalFaceOfNodes.cxx \
	SMDS_VolumeTool.cxx \
	SMDS_QuadraticEdge.cxx \
	SMDS_QuadraticFaceOfNodes.cxx \
	SMDS_QuadraticVolumeOfNodes.cxx \
	SMDS_UnstructuredGrid.cxx \
	SMDS_Downward.cxx

# additionnal information to compil and link file
libSMDS_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS)

libSMDS_la_LDFLAGS  = \
	$(VTK_LIBS) \
	$(KERNEL_LDFLAGS) -lSALOMELocalTrace -lOpUtil

# Executables targets
bin_PROGRAMS = SMDS_MemoryLimit
dist_SMDS_MemoryLimit_SOURCES = \
	SMDS_MemoryLimit.cxx

#libSMDS_la_LDFLAGS += -L/data/eap/S5_MV/INSTALL/SMESH/lib/salome -lPerfmeter
