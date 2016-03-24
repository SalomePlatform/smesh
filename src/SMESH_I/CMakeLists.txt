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
  ${OMNIORB_INCLUDE_DIR}
  ${CAS_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/Controls
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/MEDWrapper/Base
  ${PROJECT_SOURCE_DIR}/src/MEDWrapper/Factory
  ${PROJECT_SOURCE_DIR}/src/Driver
  ${PROJECT_SOURCE_DIR}/src/DriverMED
  ${PROJECT_SOURCE_DIR}/src/DriverCGNS
  ${PROJECT_SOURCE_DIR}/src/SMESH
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

IF(WIN32)
  ADD_DEFINITIONS(-DNOGDI)
ENDIF(WIN32)

# libraries to link to
SET(_link_LIBRARIES
  ${Boost_LIBRARIES}
  ${KERNEL_OpUtil}
  ${KERNEL_SalomeNotification}
  ${KERNEL_ResourcesManager}
  ${KERNEL_SalomeResourcesManager}
  ${KERNEL_SALOMEBasics}
  ${KERNEL_SalomeContainer}
  ${KERNEL_SalomeNS}
  ${KERNEL_Registry}
  ${KERNEL_SalomeHDFPersist}
  ${KERNEL_SalomeLifeCycleCORBA}
  ${KERNEL_TOOLSDS}  
  ${KERNEL_SalomeGenericObj}
  ${KERNEL_SalomeIDLKERNEL}
  ${KERNEL_SALOMELocalTrace}
  ${CAS_TKCDF}
  ${CAS_TKBO}
  ${CAS_TKShHealing}
  ${GEOM_GEOMClient}
  ${GEOM_SalomeIDLGEOM}  
  SalomeIDLSMESH
  SMESHimpl
  SMDS
  SMESHDS
  SMESHControls
  MeshDriverMED
)

# --- headers ---

# header files / no moc processing
SET(SMESHEngine_HEADERS
  SMESH_Gen_i.hxx
  SMESH_Algo_i.hxx
  SMESH_0D_Algo_i.hxx
  SMESH_1D_Algo_i.hxx
  SMESH_2D_Algo_i.hxx
  SMESH_3D_Algo_i.hxx
  SMESH_subMesh_i.hxx
  SMESH_Mesh_i.hxx
  SMESH_Hypothesis_i.hxx
  SMESH_PythonDump.hxx
  SMESH_Group_i.hxx
  SMESH_Filter_i.hxx
  SMESH_MeshEditor_i.hxx
  SMESH_Pattern_i.hxx
  SMESH_2smeshpy.hxx
  SMESH_NoteBook.hxx
  SMESH_Measurements_i.hxx
  SMESH_PreMeshInfo.hxx
  SMESH_MeshPartDS.hxx
  SMESH.hxx
)

# --- sources ---

# sources / static
SET(SMESHEngine_SOURCES
  SMESH_Gen_i.cxx
  SMESH_Gen_i_1.cxx
  SMESH_DumpPython.cxx
  SMESH_Mesh_i.cxx
  SMESH_subMesh_i.cxx
  SMESH_MeshEditor_i.cxx
  SMESH_Hypothesis_i.cxx
  SMESH_Algo_i.cxx
  SMESH_0D_Algo_i.cxx
  SMESH_1D_Algo_i.cxx
  SMESH_2D_Algo_i.cxx
  SMESH_3D_Algo_i.cxx
  SMESH_Filter_i.cxx
  SMESH_Group_i.cxx
  SMESH_Pattern_i.cxx
  SMESH_2smeshpy.cxx
  SMESH_NoteBook.cxx
  SMESH_Measurements_i.cxx
  SMESH_PreMeshInfo.cxx
)

# --- rules ---

ADD_LIBRARY(SMESHEngine ${SMESHEngine_SOURCES})
TARGET_LINK_LIBRARIES(SMESHEngine ${_link_LIBRARIES} )
INSTALL(TARGETS SMESHEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SMESHEngine_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
