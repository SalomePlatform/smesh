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

#  File   : SMESH_test1.py
#  Module : SMESH
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# ---- define a box

box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

# ---- add first face of box in study

subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
face = subShapeList[0]
name = geompy.SubShapeName(face, box)
print name
idface = geompy.addToStudyInFather(box, face, name)

# ---- add shell from box  in study

subShellList = geompy.SubShapeAll(box, geompy.ShapeType["SHELL"])
shell = subShellList[0]
name = geompy.SubShapeName(shell, box)
print name
idshell = geompy.addToStudyInFather(box, shell, name)

# ---- add first edge of face in study

edgeList = geompy.SubShapeAll(face, geompy.ShapeType["EDGE"])
edge = edgeList[0]
name = geompy.SubShapeName(edge, face)
print name
idedge = geompy.addToStudyInFather(face, edge, name)


# ---- SMESH

# ---- Init a Mesh with the box

mesh = smesh.Mesh(box, "Meshbox")

print "-------------------------- add hypothesis to box"
algoReg1 = mesh.Segment()
hypNbSeg1 = algoReg1.NumberOfSegments(7)
print hypNbSeg1.GetName()
print hypNbSeg1.GetId()
print hypNbSeg1.GetNumberOfSegments()
smesh.SetName(hypNbSeg1, "NumberOfSegments_7")

algoMef1 = mesh.Triangle()
hypArea1 = algoMef1.MaxElementArea(2500)
print hypArea1.GetName()
print hypArea1.GetId()
print hypArea1.GetMaxElementArea()
smesh.SetName(hypArea1, "MaxElementArea_2500")

# ---- add hypothesis to edge
print "-------------------------- add hypothesis to edge"
edge = salome.IDToObject(idedge)

algoReg2 = mesh.Segment(edge)
hypLen1 = algoReg2.LocalLength(100)
smesh.SetName(algoReg2.GetSubMesh(), "SubMeshEdge")
print hypLen1.GetName()
print hypLen1.GetId()
print hypLen1.GetLength()
smesh.SetName(hypLen1, "Local_Length_100")

# ---- add hypothesis to face
print "-------------------------- add hypothesis to face"
face = salome.IDToObject(idface)

algoMef2 = mesh.Triangle(face)
hypArea2 = algoMef2.MaxElementArea(500)
smesh.SetName(algoMef2.GetSubMesh(), "SubMeshFace")
print hypArea2.GetName()
print hypArea2.GetId()
print hypArea2.GetMaxElementArea()
smesh.SetName(hypArea2, "MaxElementArea_500")


salome.sg.updateObjBrowser(True)
