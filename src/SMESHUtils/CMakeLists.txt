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
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/SMDS
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKShHealing}
  ${CAS_TKPrim}
  ${CAS_TKernel}
  ${CAS_TKBRep}
  ${CAS_TKG2d}
  ${CAS_TKG3d}
  ${CAS_TKGeomBase}
  ${CAS_TKGeomAlgo}
  ${CAS_TKTopAlgo}
  ${CAS_TKMesh}
  ${Boost_LIBRARIES}
  SMDS
)

# --- headers ---

# header files / no moc processing
SET(SMESHUtils_HEADERS
  SMESH_Block.hxx
  SMESH_TypeDefs.hxx
  SMESH_Tree.hxx
  SMESH_Octree.hxx
  SMESH_Quadtree.hxx
  SMESH_OctreeNode.hxx
  SMESH_Comment.hxx
  SMESH_ComputeError.hxx
  SMESH_File.hxx
  SMESH_Utils.hxx
  SMESH_TryCatch.hxx
  SMESH_MeshAlgos.hxx
  SMESH_MAT2d.hxx
  SMESH_ControlPnt.hxx
)

# --- sources ---

# sources / static
SET(SMESHUtils_SOURCES
  SMESH_Block.cxx
  SMESH_Quadtree.cxx
  SMESH_Octree.cxx
  SMESH_OctreeNode.cxx
  SMESH_TryCatch.cxx
  SMESH_File.cxx
  SMESH_MeshAlgos.cxx
  SMESH_MAT2d.cxx
  SMESH_FreeBorders.cxx
  SMESH_ControlPnt.cxx
)

# --- rules ---

ADD_LIBRARY(SMESHUtils ${SMESHUtils_SOURCES})
TARGET_LINK_LIBRARIES(SMESHUtils ${_link_LIBRARIES} )
INSTALL(TARGETS SMESHUtils EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SMESHUtils_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
