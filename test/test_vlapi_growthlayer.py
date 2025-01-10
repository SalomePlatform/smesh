#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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

import math
import salome
salome.salome_init_without_session()

import GEOM
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder
from salome.shaper import model

def assertAlmostEqual(a,b,tol):
   if ( abs(a-b) < tol ):
      return True
   else:
      print( "not close vals", a, b )
      return False


geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)

# create a disk
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
Box = geompy.MakeBox(0,0,0,10,10,10)

smesh_builder = smeshBuilder.New()

MesherBox = smesh_builder.Mesh(Box, "Box")
viscousBuilder = MesherBox.ViscousLayerBuilder()

#############BOX AND TETRA ELEMENTS
#Set prismatic layer parameters
offset = 0.7
numLayers = 4
viscousBuilder.setBuilderParameters( offset, numLayers, 1.2, [13], False )
ShrinkBox = viscousBuilder.GetShrinkGeometry()

#Mesh the shrink box
MesherShinkBox = smesh_builder.Mesh(ShrinkBox, "ShrinkMesh")
ShrinkBoxMesh = MesherShinkBox.Tetrahedron(smeshBuilder.NETGEN_1D2D3D)

#Compute
success = MesherShinkBox.Compute()
assert( success )
assert( MesherShinkBox.NbVolumes() == 5 ) # if Fails! change the default value of volumes when meshing with Netgen!

FinalMesh = viscousBuilder.AddLayers( MesherShinkBox )
assert( FinalMesh.NbVolumes() == 5 + numLayers * 2 )    # here 2 stands for the number of face elements per face in the box
assert( FinalMesh.NbFaces() == 6 *  2 + 4 * numLayers ) # here is the number of face elements for the box + the new faces in the VL. (6 is the number of sides in the box)

#Testing the configuration where face 13 is ignored and so the offset is applied to all other faces
viscousBuilder.setBuilderParameters( offset, numLayers, 1.2, [13], True )
ShrinkBox2 = viscousBuilder.GetShrinkGeometry()
#Mesh the shrink box
MesherShinkBox2 = smesh_builder.Mesh(ShrinkBox2, "ShrinkMesh2")
ShrinkBoxMesh2 = MesherShinkBox2.Tetrahedron(smeshBuilder.NETGEN_1D2D3D)

#Compute
success = MesherShinkBox2.Compute()
assert( success )

#Test the number of elements on the shrink mesh
assert( MesherShinkBox2.NbVolumes() == 5 ) # if Fails! change the default (default hypo) number of volumes when meshing with Netgen!

FinalMesh2 = viscousBuilder.AddLayers( MesherShinkBox2 )

assert( FinalMesh2.NbVolumes() == 5 + numLayers * 2 * 5  )   # here 2 stands for the number of face elements per face in the box
assert( FinalMesh2.NbFaces() == 6 *  2 + 4 * numLayers ) # here is the number of face elements for the box + the new faces in the VL. (6 is the number of sides in the box)
#############END BOX AND TETRA ELEMENTS

#############MESH SQUARE FACE
Face = geompy.MakeFaceHW(5, 5, 1)
Disk = geompy.MakeDiskR(5, 1)

MesherSqr = smesh_builder.Mesh(Face, "Face")
viscousBuilder = MesherSqr.ViscousLayerBuilder()
#Set prismatic layer parameters
offset = 0.5
numberOfLayers = 6
viscousBuilder.setBuilderParameters( offset, numberOfLayers, 1.2 )
ShrinkFace = viscousBuilder.GetShrinkGeometry()
#Mesh the shrink face
MesherShinkFace = smesh_builder.Mesh(ShrinkFace, "ShrinkFaceMesh")
algo = MesherShinkFace.Segment()
numOfSegments = 4
algo.NumberOfSegments(numOfSegments)
ShrinkFaceMesh = MesherShinkFace.Triangle()
#Compute
success = MesherShinkFace.Compute()
assert( success )
numFaceElementShrinkGeom = MesherShinkFace.NbFaces()
FinalFaceMesh = viscousBuilder.AddLayers( MesherShinkFace )
# Check the number of additional elements
# numOfSegments * 4 * numberOfLayers
finalNumOfElements = FinalFaceMesh.NbFaces()
assert( numFaceElementShrinkGeom + 4 * numOfSegments * numberOfLayers == finalNumOfElements )

#############END MESH SQUARE FACE

#############MESH CIRCULAR FACE
MesherCircle = smesh_builder.Mesh(Disk, "Disk")
viscousBuilder = MesherCircle.ViscousLayerBuilder()
viscousBuilder.setBuilderParameters( offset, numberOfLayers, 1.2 )
ShrinkCircle = viscousBuilder.GetShrinkGeometry()
MesherShinkCircle = smesh_builder.Mesh(ShrinkCircle, "ShrinkCircleMesh")
algo = MesherShinkCircle.Segment()
numOfSegments = 12
algo.NumberOfSegments(numOfSegments)
ShrinkCircleMesh = MesherShinkCircle.Triangle()

#Compute
success = MesherShinkCircle.Compute()
numFaceElementShrinkGeom = MesherShinkCircle.NbFaces()
assert( success )
FinalCircleMesh = viscousBuilder.AddLayers( MesherShinkCircle )
finalNumOfElements = FinalCircleMesh.NbFaces()
assert( numFaceElementShrinkGeom + numOfSegments * numberOfLayers == finalNumOfElements )
#############END MESH CIRCULAR FACE
