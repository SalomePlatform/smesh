#  SMESH SMDS : implementaion of Salome mesh data structure
#
#  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
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
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#
#
#  File   : Makefile.in
#  Author : Patrick GOLDBRONN (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH

include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMDS_TypeOfPosition.hxx \
	SMDSAbs_ElementType.hxx \
	SMDS_EdgePosition.hxx \
	SMDS_ElemIterator.hxx \
	SMDS_FacePosition.hxx \
	SMDS_Mesh.hxx \
	SMDS_MeshEdge.hxx \
	SMDS_MeshElement.hxx \
	SMDS_MeshElementIDFactory.hxx \
	SMDS_MeshFace.hxx \
	SMDS_MeshGroup.hxx \
	SMDS_MeshIDFactory.hxx \
	SMDS_MeshNode.hxx \
	SMDS_MeshObject.hxx \
	SMDS_MeshVolume.hxx \
	SMDS_Position.hxx \
	SMDS_SpacePosition.hxx \
	SMDS_VertexPosition.hxx \
	SMDS_Iterator.hxx \
	SMDS_IteratorOfElements.hxx \
	SMDS_VolumeOfFaces.hxx \
	SMDS_VolumeOfNodes.hxx \
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
	SMDS_MeshInfo.hxx


# Libraries targets

lib_LTLIBRARIES = libSMDS.la
dist_libSMDS_la_SOURCES = \
	SMDS_MeshObject.cxx \
	SMDS_MeshElement.cxx \
	SMDS_Position.cxx \
	SMDS_EdgePosition.cxx \
	SMDS_FacePosition.cxx \
	SMDS_SpacePosition.cxx \
	SMDS_VertexPosition.cxx \
	SMDS_MeshNode.cxx \
	SMDS_MeshEdge.cxx \
	SMDS_MeshFace.cxx \
	SMDS_MeshVolume.cxx \
	SMDS_MeshElementIDFactory.cxx \
	SMDS_MeshGroup.cxx \
	SMDS_MeshIDFactory.cxx \
	SMDS_Mesh.cxx \
	SMDS_IteratorOfElements.cxx \
	SMDS_VolumeOfFaces.cxx \
	SMDS_VolumeOfNodes.cxx \
	SMDS_PolyhedralVolumeOfNodes.cxx \
	SMDS_FaceOfEdges.cxx \
	SMDS_FaceOfNodes.cxx \
	SMDS_PolygonalFaceOfNodes.cxx \
	SMDS_VolumeTool.cxx \
	SMDS_QuadraticEdge.cxx \
	SMDS_QuadraticFaceOfNodes.cxx \
	SMDS_QuadraticVolumeOfNodes.cxx

# additionnal information to compil and link file
libSMDS_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
	$(BOOST_CPPFLAGS)

libSMDS_la_LDFLAGS  = \
	$(KERNEL_LDFLAGS) -lSALOMELocalTrace \
	$(CAS_KERNEL)

# Executables targets
bin_PROGRAMS = SMDS_MemoryLimit
dist_SMDS_MemoryLimit_SOURCES = \
	SMDS_MemoryLimit.cxx

SMDS_MemoryLimit_LDADD = \
	$(KERNEL_LDFLAGS) -lSALOMELocalTrace
