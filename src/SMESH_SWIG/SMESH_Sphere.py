#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#  GEOM GEOM_SWIG : binding of C++ omplementaion with Python
#  File   : GEOM_Sphere.py
#  Author : Damien COQUERET, Open CASCADE
#  Module : GEOM
#  $Header: 
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

import math

# It is an example of creating a hexahedrical mesh on a sphere.
#
# Used approach allows to avoid problems with degenerated and
# seam edges without special processing of geometrical shapes

#-----------------------------------------------------------------------
#Variables
Radius  = 100.
Dist    = Radius / 2.
Factor  = 2.5
Angle90 = math.pi / 2.
NbSeg   = 10

PointsList = []
ShapesList = []

#Basic Elements
P0 = geompy.MakeVertex(0., 0., 0.)
P1 = geompy.MakeVertex(-Dist, -Dist, -Dist)
P2 = geompy.MakeVertex(-Dist, -Dist, Dist)
P3 = geompy.MakeVertex(-Dist, Dist, Dist)
P4 = geompy.MakeVertex(-Dist, Dist, -Dist)

VZ = geompy.MakeVectorDXDYDZ(0., 0., 1.)

#Construction Elements
PointsList.append(P1)
PointsList.append(P2)
PointsList.append(P3)
PointsList.append(P4)
PointsList.append(P1)

PolyLine = geompy.MakePolyline(PointsList)

Face1 = geompy.MakeFace(PolyLine, 1)
Face2 = geompy.MakeScaleTransform(Face1, P0, Factor)
Face3 = geompy.MakeScaleTransform(Face1, P0, -1.)

#Models
Sphere = geompy.MakeSphereR(Radius)

Block = geompy.MakeHexa2Faces(Face1, Face2)
Cube  = geompy.MakeHexa2Faces(Face1, Face3)

Common1 = geompy.MakeBoolean(Sphere, Block, 1)
Common2 = geompy.MakeRotation(Common1, VZ, Angle90)

MultiBlock1 = geompy.MakeMultiTransformation1D(Common1, 20, -1, 3)
MultiBlock2 = geompy.MakeMultiTransformation1D(Common2, 30, -1, 3)

#Reconstruct sphere from several blocks
ShapesList.append(Cube)
ShapesList.append(MultiBlock1)
ShapesList.append(MultiBlock2)
Compound = geompy.MakeCompound(ShapesList)

Result = geompy.MakeGlueFaces(Compound, 0.1)

#addToStudy
Id_Sphere      = geompy.addToStudy(Sphere, "Sphere")
Id_Cube        = geompy.addToStudy(Cube, "Cube")

Id_Common1     = geompy.addToStudy(Common1, "Common1")
Id_Common2     = geompy.addToStudy(Common2, "Common2")

Id_MultiBlock1 = geompy.addToStudy(MultiBlock1, "MultiBlock1")
Id_MultiBlock2 = geompy.addToStudy(MultiBlock2, "MultiBlock2")

Id_Result      = geompy.addToStudy(Result, "Result")

#-----------------------------------------------------------------------
#Meshing
my_hexa = smesh.Mesh(Result, "Sphere_Mesh")
algo = my_hexa.Segment()
algo.NumberOfSegments(NbSeg)
my_hexa.Quadrangle()
my_hexa.Hexahedron()
my_hexa.Compute()

salome.sg.updateObjBrowser(True)
