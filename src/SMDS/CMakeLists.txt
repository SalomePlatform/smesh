# Copyright (C) 2012-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${VTK_LIBRARIES}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_OpUtil}
)

# --- headers ---

# header files / no moc processing
SET(SMDS_HEADERS
  ObjectPool.hxx
  SMDS_TypeOfPosition.hxx
  SMDSAbs_ElementType.hxx
  SMDS_EdgePosition.hxx
  SMDS_ElemIterator.hxx
  SMDS_FacePosition.hxx
  SMDS_Mesh.hxx
  SMDS_Mesh0DElement.hxx
  SMDS_LinearEdge.hxx
  SMDS_MeshEdge.hxx
  SMDS_MeshElement.hxx
  SMDS_MeshElementIDFactory.hxx
  SMDS_MeshCell.hxx
  SMDS_MeshFace.hxx
  SMDS_MeshGroup.hxx
  SMDS_MeshIDFactory.hxx
  SMDS_MeshNode.hxx
  SMDS_MeshNodeIDFactory.hxx
  SMDS_MeshObject.hxx
  SMDS_MeshVolume.hxx
  SMDS_Position.hxx
  SMDS_SpacePosition.hxx
  SMDS_VertexPosition.hxx
  SMDS_Iterator.hxx
  SMDS_IteratorOfElements.hxx
  SMDS_VolumeOfFaces.hxx
  SMDS_VolumeOfNodes.hxx
  SMDS_VtkEdge.hxx
  SMDS_VtkFace.hxx
  SMDS_VtkVolume.hxx
  SMDS_VtkCellIterator.hxx
  SMDS_PolyhedralVolumeOfNodes.hxx
  SMDS_FaceOfEdges.hxx
  SMDS_FaceOfNodes.hxx
  SMDS_PolygonalFaceOfNodes.hxx
  SMDS_VolumeTool.hxx
  SMDS_QuadraticEdge.hxx
  SMDS_QuadraticFaceOfNodes.hxx
  SMDS_QuadraticVolumeOfNodes.hxx
  SMDS_SetIterator.hxx
  SMESH_SMDS.hxx
  SMDS_MeshInfo.hxx
  SMDS_UnstructuredGrid.hxx
  SMDS_Downward.hxx
  SMDS_StdIterator.hxx
  SMDS_IteratorOnIterators.hxx
  SMDS_BallElement.hxx
)

# --- sources ---

# sources / static
SET(SMDS_SOURCES
  chrono.cxx
  SMDS_MeshObject.cxx
  SMDS_MeshElement.cxx
  SMDS_MeshCell.cxx
  SMDS_Position.cxx
  SMDS_EdgePosition.cxx
  SMDS_FacePosition.cxx
  SMDS_SpacePosition.cxx
  SMDS_VertexPosition.cxx
  SMDS_MeshNode.cxx
  SMDS_Mesh0DElement.cxx
  SMDS_LinearEdge.cxx
  SMDS_MeshEdge.cxx
  SMDS_MeshFace.cxx
  SMDS_MeshVolume.cxx
  SMDS_MeshNodeIDFactory.cxx
  SMDS_MeshElementIDFactory.cxx
  SMDS_MeshGroup.cxx
  SMDS_MeshIDFactory.cxx
  SMDS_Mesh.cxx
  SMDS_IteratorOfElements.cxx
  SMDS_VolumeOfFaces.cxx
  SMDS_VolumeOfNodes.cxx
  SMDS_VtkEdge.cxx
  SMDS_VtkFace.cxx
  SMDS_VtkVolume.cxx
  SMDS_VtkCellIterator.cxx
  SMDS_PolyhedralVolumeOfNodes.cxx
  SMDS_FaceOfEdges.cxx
  SMDS_FaceOfNodes.cxx
  SMDS_PolygonalFaceOfNodes.cxx
  SMDS_VolumeTool.cxx
  SMDS_QuadraticEdge.cxx
  SMDS_QuadraticFaceOfNodes.cxx
  SMDS_QuadraticVolumeOfNodes.cxx
  SMDS_UnstructuredGrid.cxx
  SMDS_Downward.cxx
  SMDS_BallElement.cxx 
)

# bin programs
SET(SMDS_MemoryLimit_SOURCES
  SMDS_MemoryLimit.cxx
)

# --- rules ---

ADD_LIBRARY(SMDS ${SMDS_SOURCES})
TARGET_LINK_LIBRARIES(SMDS ${_link_LIBRARIES} )
INSTALL(TARGETS SMDS EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

ADD_EXECUTABLE(SMDS_MemoryLimit ${SMDS_MemoryLimit_SOURCES})
TARGET_LINK_LIBRARIES(SMDS_MemoryLimit ${_link_LIBRARIES})
INSTALL(TARGETS SMDS_MemoryLimit EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_BINS})

INSTALL(FILES ${SMDS_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
