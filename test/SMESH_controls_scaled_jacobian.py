#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2016-2025  CEA/DEN, EDF R&D
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

#  File   : SMESH_controls_scaled_jacobian.py
#  Author : Cesar Conopoima
#  Module : SMESH
#
from salome.kernel import salome
import math
salome.salome_init_without_session()

from salome.kernel import GEOM
from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

def assertWithDelta( refval, testvals, delta ):
  return ( refval <= testvals+delta and refval >= testvals-delta ) 

geompy = geomBuilder.New()
smesh_builder = smeshBuilder.New()

Box_1 = geompy.MakeBoxDXDYDZ(10, 10, 10)
geompy.addToStudy( Box_1, 'Box_1' )

smesh = smeshBuilder.New()

Mesh_1 = smesh.Mesh(Box_1,'Mesh_1')
NETGEN_1D_2D_3D = Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
Done = Mesh_1.Compute()

if not Done:
  raise Exception("Error when computing NETGEN_1D2D3D Mesh for quality control test")

#For tetra elements
perfect   = 1.0
externals = math.sqrt( 2.0 )/2.0
notPerfectElements  = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_ScaledJacobian, SMESH.FT_LessThan, perfect - 1e-12 )
perfectElements     = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_ScaledJacobian, SMESH.FT_EqualTo, perfect )
externalElements    = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_ScaledJacobian, SMESH.FT_EqualTo, externals )

notPerfectIds = Mesh_1.GetIdsFromFilter(notPerfectElements)
perfectIds    = Mesh_1.GetIdsFromFilter(perfectElements)
externalsIds  = Mesh_1.GetIdsFromFilter(externalElements)

assert( len(notPerfectIds)  == 4 )
assert( len(perfectIds)     == 1 )
assert( len(externalsIds)   == 4 )

# Test GetScaledJacobian by elementId
for id in range(len(perfectIds)):
  assert( assertWithDelta( perfect, Mesh_1.GetScaledJacobian( perfectIds[ id ] ), 1e-12) )

for id in range(len(externalsIds)):
  assert( assertWithDelta( externals, Mesh_1.GetScaledJacobian( externalsIds[ id ] ), 1e-12) )

#For hexa elements
Mesh_2 = smesh.Mesh(Box_1,'Mesh_2')
Cartesian_3D = Mesh_2.BodyFitted()
Body_Fitting_Parameters_1 = Cartesian_3D.SetGrid([ [ '5.0' ], [ 0, 1 ]],[ [ '5.0' ], [ 0, 1 ]],[ [ '5.0' ], [ 0, 1 ]],4,0)
Done = Mesh_2.Compute()

if not Done:
  raise Exception("Error when computing BodyFitted Mesh for quality control test")

notPerfectIds = Mesh_2.GetIdsFromFilter(notPerfectElements)
perfectIds    = Mesh_2.GetIdsFromFilter(perfectElements)

assert( len(notPerfectIds)  == 0 )
assert( len(perfectIds)     == 8 )

# Test GetScaledJacobian by elementId
for id in range(len(perfectIds)):
  assert( assertWithDelta( perfect, Mesh_2.GetScaledJacobian( perfectIds[ id ] ), 1e-12) )

#For hexa elements with poor quality
Mesh_3 = smesh.Mesh(Box_1,'Mesh_3')
Cartesian_3D = Mesh_3.BodyFitted()
Body_Fitting_Parameters_1 = Cartesian_3D.SetGrid([ [ '5.0' ], [ 0, 1 ]],[ [ '5.0' ], [ 0, 1 ]],[ [ '5.0' ], [ 0, 1 ]],4,0)
Body_Fitting_Parameters_1.SetAxesDirs( SMESH.DirStruct( SMESH.PointStruct ( 1, 0, 1 )), SMESH.DirStruct( SMESH.PointStruct ( 0, 1, 0 )), SMESH.DirStruct( SMESH.PointStruct ( 0, 0, 1 )) )
Done = Mesh_3.Compute()

if not Done:
  raise Exception("Error when computing BodyFitted Distorted Mesh for quality control test")

#Polyhedrons return zero scaled jacobian because of lack for a decomposition into simpler forms
Polys   = 0.0
#Hexahedrons that are distorted by an angle of 45 
# Scaled Jacobian which is a measure of elements distortion 
# will return cos(45) = math.sqrt( 2.0 )/2.0 
distorted = math.sqrt( 2.0 )/2.0 
polysElements     = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_ScaledJacobian, SMESH.FT_EqualTo, Polys )
distortedElements = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_ScaledJacobian, SMESH.FT_EqualTo, distorted )

polysIds        = Mesh_3.GetIdsFromFilter(polysElements)
distortedIds    = Mesh_3.GetIdsFromFilter(distortedElements)

assert( len(polysIds)     == 4 )
assert( len(distortedIds) == 8 )

# Test GetScaledJacobian by elementId
for id in range(len(distortedIds)):
  assert( assertWithDelta( distorted, Mesh_3.GetScaledJacobian( distortedIds[ id ] ), 1e-12 ) )

#Test the pentahedron
Mesh_4 = smesh.Mesh(Box_1,'Mesh_4')
Cartesian_3D = Mesh_4.BodyFitted()
Body_Fitting_Parameters_1 = Cartesian_3D.SetGrid([ [ '4' ], [ 0, 1 ]],[ [ '4' ], [ 0, 1 ]],[ [ '4' ], [ 0, 1 ]],4,0)
Body_Fitting_Parameters_1.SetAxesDirs( SMESH.DirStruct( SMESH.PointStruct ( 1, 0, 1 )), SMESH.DirStruct( SMESH.PointStruct ( 0, 1, 0 )), SMESH.DirStruct( SMESH.PointStruct ( 0, 0, 1 )) )
Body_Fitting_Parameters_1.SetSizeThreshold( 4 )
Body_Fitting_Parameters_1.SetToAddEdges( 0 )
Body_Fitting_Parameters_1.SetGridSpacing( [ '2' ], [ 0, 1 ], 0 )
Body_Fitting_Parameters_1.SetGridSpacing( [ '2' ], [ 0, 1 ], 1 )
Body_Fitting_Parameters_1.SetGridSpacing( [ '2' ], [ 0, 1 ], 2 )
Done = Mesh_4.Compute()

if not Done:
  raise Exception("Error when computing BodyFitted Distorted Mesh for quality control test")

pentahedrons = 0.6
pentasAndPolys     = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_ScaledJacobian, SMESH.FT_LessThan, pentahedrons )

polysIds          = Mesh_4.GetIdsFromFilter(polysElements)
pentasAndPolysIds = Mesh_4.GetIdsFromFilter(pentasAndPolys)

assert( len(pentasAndPolysIds) - len(polysIds) == 10 )

#Test distorded hexahedrons scaled jacobian values
Mesh_5 = smesh.Mesh(Box_1,'Mesh_5')
Regular_1D = Mesh_5.Segment()
Number_of_Segments_1 = Regular_1D.NumberOfSegments(2)
Quadrangle_2D = Mesh_5.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = Mesh_5.Hexahedron(algo=smeshBuilder.Hexa)
isDone = Mesh_5.Compute()

if not Done:
  raise Exception("Error when computing hexaedrons Mesh for quality control test")

#move some nodes to make scaled jacobian lesser than 1
node_id_1 = Mesh_5.FindNodeClosestTo(0, 0, 10)
node_id_5 = Mesh_5.FindNodeClosestTo(10, 0, 10)
node_id_14 = Mesh_5.FindNodeClosestTo(10, 5, 10)
node_id_13 = Mesh_5.FindNodeClosestTo(10, 0, 5)
node_id_6 = Mesh_5.FindNodeClosestTo(10, 0, 0)
Mesh_5.MoveNode( node_id_1, 1, 1, 9 )
Mesh_5.MoveNode( node_id_5, 9, 1, 9 )
Mesh_5.MoveNode( node_id_14, 10, 5, 9 )
Mesh_5.MoveNode( node_id_13, 9, 0, 5 )
Mesh_5.MoveNode( node_id_6, 8, 0, 0 )

yellow_element = Mesh_5.FindElementsByPoint(7.5, 2.5, 2.5)[0]
green_element = Mesh_5.FindElementsByPoint(7.5, 2.5, 7.5)[0]
blue_element = Mesh_5.FindElementsByPoint(2.5, 2.5, 7.5)[0]

yellow_SJ = Mesh_5.GetScaledJacobian(yellow_element)
green_SJ = Mesh_5.GetScaledJacobian(green_element)
blue_SJ = Mesh_5.GetScaledJacobian(blue_element)

yellow_SJ_ref = 0.910446300912
green_SJ_ref = 0.818025491961
blue_SJ_ref = 0.654728501099

assert assertWithDelta( yellow_SJ_ref, yellow_SJ, 1e-10 )
assert assertWithDelta( green_SJ_ref, green_SJ, 1e-10 )
assert assertWithDelta( blue_SJ_ref, blue_SJ, 1e-10 )
