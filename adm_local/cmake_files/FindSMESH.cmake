# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

SET(SMESH_CXXFLAGS -I${SMESH_ROOT_DIR}/include/salome)

FIND_LIBRARY(GeomSelectionTools GeomSelectionTools ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(MEFISTO2D MEFISTO2D ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(MeshDriverDAT MeshDriverDAT ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(MeshDriverMED MeshDriverMED ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(MeshDriver MeshDriver ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(MeshDriverSTL MeshDriverSTL ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(MeshDriverUNV MeshDriverUNV ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SalomeIDLSMESH SalomeIDLSMESH ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMDS SMDS ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESHClient SMESHClient ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESHControls SMESHControls ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESHDS SMESHDS ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESHEngine SMESHEngine ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESHFiltersSelection SMESHFiltersSelection ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESHimpl SMESHimpl ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESHObject SMESHObject ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESH SMESH ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(StdMeshersEngine StdMeshersEngine ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(StdMeshersGUI StdMeshersGUI ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(StdMeshers StdMeshers ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(SMESHUtils SMESHUtils ${SMESH_ROOT_DIR}/lib/salome)
FIND_LIBRARY(MeshDriverGMF MeshDriverGMF ${SMESH_ROOT_DIR}/lib/salome)
