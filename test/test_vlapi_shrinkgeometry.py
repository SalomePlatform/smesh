#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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

#Set prismatic layer parameters
offset = 0.1

####SHRINK THE BOX IN ALL DIRECTIONS
#No list of faces is passed and the isToIgnore flag true by default so the offset if applied to the entire geometry
viscousBuilder.setBuilderParameters( offset, 4, 1.2 )
ShrinkBox = viscousBuilder.GetShrinkGeometry()

BoxProperties = geompy.BasicProperties(Box)
ShrinkBoxProperties = geompy.BasicProperties(ShrinkBox)
assert( BoxProperties[2] > ShrinkBoxProperties[2] )

assert( assertAlmostEqual( BoxProperties[2], (10.0)**(3.0), 1e-12 ) )
#The geometry is shrank in all directions
assert( assertAlmostEqual( ShrinkBoxProperties[2], (10.0-offset*2)**(3), 1e-12 ) )
####END SHRINK THE BOX IN ALL DIRECTIONS

####SHRINK THE BOX EXCEPT FOR ONE FACE
viscousBuilder = MesherBox.ViscousLayerBuilder()
selectableFaces = geompy.SubShapeAllSortedCentresIDs(Box, geompy.ShapeType["FACE"])
# Set face 1 TO BE ignored
viscousBuilder.setBuilderParameters( offset, 4, 1.2, [ selectableFaces[ 0 ] ], True  ) # Shrink in all faces except face id
ShrinkBox = viscousBuilder.GetShrinkGeometry()
ShrinkBoxProperties = geompy.BasicProperties(ShrinkBox)
selectableShrinkFaces = geompy.SubShapeAllSortedCentresIDs(ShrinkBox, geompy.ShapeType["FACE"])
assert( assertAlmostEqual( ShrinkBoxProperties[2], (10.0-offset*2)**(2)*(10.0-offset), 1e-12 ) )
####END SHRINK THE BOX EXCEPT FOR ONE FACE

####SHRINK THE BOX IN DIRECTION OF ONLY ONE FACE
# Set face 1 TO NOT be ignored
viscousBuilder.setBuilderParameters( offset, 4, 1.2, [ selectableFaces[ 0 ] ], False ) # Shrink only the faceid
ShrinkBox = viscousBuilder.GetShrinkGeometry()
ShrinkBoxProperties = geompy.BasicProperties(ShrinkBox)
assert( assertAlmostEqual( ShrinkBoxProperties[2], (10.0)**(2)*(10.0-offset), 1e-12 ) )
selectableShrinkFaces = geompy.SubShapeAllSortedCentresIDs(ShrinkBox, geompy.ShapeType["FACE"])
####END SHRINK THE BOX IN DIRECTION OF ONLY ONE FACE

####DO NOT SHRINK THE BOX
viscousBuilder.setBuilderParameters( offset, 4, 1.2, isElementToIgnore = False )
ShrinkBox = viscousBuilder.GetShrinkGeometry()
BoxProperties = geompy.BasicProperties(Box)
ShrinkBoxProperties = geompy.BasicProperties(ShrinkBox)
assert( assertAlmostEqual( BoxProperties[2], ShrinkBoxProperties[2], 1e-12) )
####END DO NOT SHRINK THE BOX

####SHRINK THE ENTIRE SPHERE
#Test shrinking sphere
Radius = 10.0
Sphere = geompy.MakeSphere(0,0,0,Radius)
MesherSphere = smesh_builder.Mesh(Sphere, "Sphere")
viscousBuilder = MesherSphere.ViscousLayerBuilder()
viscousBuilder.setBuilderParameters( offset, 4, 1.2 )
ShrinkSphere = viscousBuilder.GetShrinkGeometry()
ShrinkSphereProperties = geompy.BasicProperties(ShrinkSphere)
assert( ShrinkSphereProperties[2] < 4.0/3.0*math.pi * Radius**3 )
assert( assertAlmostEqual( ShrinkSphereProperties[2], 4.0/3.0*math.pi*(10.0-offset)**(3), 1e-12 ) )
####END SHRINK THE ENTIRE SPHERE

####SHRINK THE ENTIRE CYLINDER
#Test shrinking cylinder
Cylinder = geompy.MakeCylinderRH(10,30)
MesherCylinder = smesh_builder.Mesh(Cylinder, "Cylinder")
viscousBuilder = MesherCylinder.ViscousLayerBuilder()
viscousBuilder.setBuilderParameters( offset, 4, 1.2 )
ShrinkCylinder = viscousBuilder.GetShrinkGeometry()
CylinderProp = geompy.BasicProperties(Cylinder)
ShirnkCylinderProp = geompy.BasicProperties(ShrinkCylinder)

assert( CylinderProp[2] > ShirnkCylinderProp[2] )
####END SHRINK THE ENTIRE CYLINDER

####SHRINK THE ENTIRE TUBE
#Test shrinking tube
Circle_1 = geompy.MakeCircle(None, None, 20)
Circle_2 = geompy.MakeCircle(None, None, 10)
Face_1 = geompy.MakeFaceWires([Circle_1, Circle_2], 1)
Tube = geompy.MakePrismDXDYDZ(Face_1, 0, 0, 100)

MesherTube = smesh_builder.Mesh(Tube, "Tube")
viscousBuilder = MesherTube.ViscousLayerBuilder()
viscousBuilder.setBuilderParameters( offset, 4, 1.2 )
ShrinkTube = viscousBuilder.GetShrinkGeometry()
TubeProp = geompy.BasicProperties(Tube)
ShirnkTubeProp = geompy.BasicProperties(ShrinkTube)
assert( TubeProp[2] > ShirnkTubeProp[2] )
####END SHRINK THE ENTIRE TUBE

####SHRINK COMPOUND OBJECT TO GENERATE COMPOUND WITH COMMON FACE
X     = geompy.MakeVectorDXDYDZ( 1,0,0 )
O     = geompy.MakeVertex( 100,50,50 )
plane = geompy.MakePlane( O, X, 200 ) # plane YZ
lX    = 200
lYlZ  = 100
box   = geompy.MakeBoxDXDYDZ(lX,lYlZ,lYlZ)
sBox  = geompy.MakeHalfPartition( box, plane )

# Generate a uniquebody whit coincident faces
# 4 left, 34 middle, 50 right
ignoreFaces = [4,34,50]
geompy.addToStudy( sBox, "SisterBox" )
MesherSBox = smesh_builder.Mesh( sBox, "SisterBoxMesh")
ViscousBuilder = MesherSBox.ViscousLayerBuilder()
thickness = 20
numberOfLayers = 10
stretchFactor = 1.5
ViscousBuilder.setBuilderParameters( thickness, numberOfLayers, stretchFactor, ignoreFaces )
ShrinkSBox = ViscousBuilder.GetShrinkGeometry()
SBoxProp = geompy.BasicProperties(sBox)
ShirnksBoxProp = geompy.BasicProperties(ShrinkSBox)
assert( assertAlmostEqual(ShirnksBoxProp[2], lX * (lYlZ - 2.0*thickness)**(2.0), 1e-12 ) )
####END SHRINK COMPOUND OBJECT TO GENERATE COMPUND WITH COMMON FACE


####SHRINK COMPOUND OBJECT TO GENERATE TWO DISJOINT SOLIDS
ignoreFaces = [4,50]
ViscousBuilder.setBuilderParameters( thickness, numberOfLayers, stretchFactor, ignoreFaces )
ShrinkSBox2 = ViscousBuilder.GetShrinkGeometry()
SBoxProp = geompy.BasicProperties(sBox)
ShirnksBoxProp2 = geompy.BasicProperties(ShrinkSBox2)
assert( assertAlmostEqual(ShirnksBoxProp2[2], (lX -2.0*thickness) * (lYlZ - 2.0*thickness)**(2.0), 1e-12 ) )
####END SHRINK COMPOUND OBJECT TO GENERATE TWO DISJOINT SOLIDS


######SHRINK SQUARE
offset = 0.5
numberOfLayers = 6
Face = geompy.MakeFaceHW(5, 5, 1)
MesherSqr = smesh_builder.Mesh(Face, "Face")
viscousBuilder = MesherSqr.ViscousLayerBuilder()
viscousBuilder.setBuilderParameters( offset, numberOfLayers, 1.2 )
ShrinkFace = viscousBuilder.GetShrinkGeometry()

FaceProperties = geompy.BasicProperties(Face)
ShrinkFaceProperties = geompy.BasicProperties(ShrinkFace)
#Test smaller face
assert( ShrinkFaceProperties[1] < FaceProperties[1] )
assertAlmostEqual( ShrinkFaceProperties[1], (5.0-offset*2.0)**(2.0), 1e-12 )
######END SHRINK SQUARE

######SHRINK CIRCLE
Disk = geompy.MakeDiskR(5, 1)

#Test with circle
MesherCircle = smesh_builder.Mesh(Disk, "Disk")
viscousBuilder = MesherCircle.ViscousLayerBuilder()
viscousBuilder.setBuilderParameters( offset, numberOfLayers, 1.2 )
ShrinkCircle = viscousBuilder.GetShrinkGeometry()
FaceProperties = geompy.BasicProperties(Disk)
ShrinkFaceProperties = geompy.BasicProperties(ShrinkCircle)

assert( ShrinkFaceProperties[1] < FaceProperties[1] )
######END SHRINK CIRCLE
