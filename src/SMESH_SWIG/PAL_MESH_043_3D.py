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

#  File        : SMESH_testExtrusion3D.py
#  Module      : SMESH
#  Description : Create meshes to test extrusion of mesh elements along path
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# create points to build two circles
p1 = geompy.MakeVertex(0,  100,  0)
p2 = geompy.MakeVertex(100,  0,  0)
p3 = geompy.MakeVertex(0, -100,  0)
p4 = geompy.MakeVertex(0,   70,  0)
p5 = geompy.MakeVertex(0,  100, 30)
p6 = geompy.MakeVertex(0,  130,  0)

# create two circles
circle = geompy.MakeCircleThreePnt(p1, p2, p3)
cf     = geompy.MakeCircleThreePnt(p4, p5, p6)

# make circular face
wire = geompy.MakeWire([cf])
face = geompy.MakeFace(wire, 1)

# publish circular face and second circle
idcircle = geompy.addToStudy(circle, "Circle")
idface   = geompy.addToStudy(face,   "Circular face")


# init a Mesh with the circular face
mesh1 = smesh.Mesh(face, "Mesh on circular face")

# set hypotheses and algos to the first mesh
numberOfSegments1 = 12
algoReg1 = mesh1.Segment()
algoReg1.SetName("Regular_1D")
hypNbSeg1 = algoReg1.NumberOfSegments(numberOfSegments1)
smesh.SetName(hypNbSeg1, "NumberOfSegments_" + str(numberOfSegments1))

maxElementArea = 30

algoMef = mesh1.Triangle()
algoMef.SetName("MEFISTO_2D")
hypArea = algoMef.MaxElementArea(maxElementArea)
smesh.SetName(hypArea, "MaxElementArea_" + str(maxElementArea))


# init a Mesh with the second circle
mesh2 = smesh.Mesh(circle, "Mesh on circular edge")

numberOfSegments2 = 12
algoReg2 = mesh2.Segment()
algoReg2.SetName("Regular_1D")
hypNbSeg2 = algoReg2.NumberOfSegments(numberOfSegments2)
smesh.SetName(hypNbSeg2, "NumberOfSegments_" + str(numberOfSegments2))


# compute meshes
mesh1.Compute()
mesh2.Compute()

# ---- update object browser
salome.sg.updateObjBrowser(True)
