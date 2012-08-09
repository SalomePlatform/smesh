#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#  GEOM GEOM_SWIG : binding of C++ omplementaion with Python
#  File   : GEOM_Sphere.py
#  Author : Damien COQUERET, Open CASCADE
#  Module : GEOM
#  $Header: 
#
from geompy import *
from math import *

import smesh

# It is an example of creating a hexahedrical mesh on a sphere.
#
# Used approach allows to avoid problems with degenerated and
# seam edges without special processing of geometrical shapes

#-----------------------------------------------------------------------
#Variables
Radius  = 100.
Dist    = Radius / 2.
Factor  = 2.5
Angle90 = pi / 2.
NbSeg   = 10

PointsList = []
ShapesList = []

#Basic Elements
P0 = MakeVertex(0., 0., 0.)
P1 = MakeVertex(-Dist, -Dist, -Dist)
P2 = MakeVertex(-Dist, -Dist, Dist)
P3 = MakeVertex(-Dist, Dist, Dist)
P4 = MakeVertex(-Dist, Dist, -Dist)

VZ = MakeVectorDXDYDZ(0., 0., 1.)

#Construction Elements
PointsList.append(P1)
PointsList.append(P2)
PointsList.append(P3)
PointsList.append(P4)
PointsList.append(P1)

PolyLine = MakePolyline(PointsList)

Face1 = MakeFace(PolyLine, 1)
Face2 = MakeScaleTransform(Face1, P0, Factor)
Face3 = MakeScaleTransform(Face1, P0, -1.)

#Models
Sphere = MakeSphereR(Radius)

Block = MakeHexa2Faces(Face1, Face2)
Cube  = MakeHexa2Faces(Face1, Face3)

Common1 = MakeBoolean(Sphere, Block, 1)
Common2 = MakeRotation(Common1, VZ, Angle90)

MultiBlock1 = MakeMultiTransformation1D(Common1, 20, -1, 3)
MultiBlock2 = MakeMultiTransformation1D(Common2, 30, -1, 3)

#Reconstruct sphere from several blocks
ShapesList.append(Cube)
ShapesList.append(MultiBlock1)
ShapesList.append(MultiBlock2)
Compound = MakeCompound(ShapesList)

Result = MakeGlueFaces(Compound, 0.1)

#addToStudy
Id_Sphere      = addToStudy(Sphere, "Sphere")
Id_Cube        = addToStudy(Cube, "Cube")

Id_Common1     = addToStudy(Common1, "Common1")
Id_Common2     = addToStudy(Common2, "Common2")

Id_MultiBlock1 = addToStudy(MultiBlock1, "MultiBlock1")
Id_MultiBlock2 = addToStudy(MultiBlock2, "MultiBlock2")

Id_Result      = addToStudy(Result, "Result")

#-----------------------------------------------------------------------
#Meshing
smesh.SetCurrentStudy(salome.myStudy)
my_hexa = smesh.Mesh(Result, "Sphere_Mesh")
algo = my_hexa.Segment()
algo.NumberOfSegments(NbSeg)
my_hexa.Quadrangle()
my_hexa.Hexahedron()
my_hexa.Compute()
