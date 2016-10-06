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

# Tetrahedrization of the geometry union of 2 boxes having a face in common
# Hypothesis and algorithms for the mesh generation are global
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# ---- define 2 boxes box1 and box2

box1 = geompy.MakeBox(0., 0., 0., 100., 200., 300.)

idbox1 = geompy.addToStudy(box1, "box1")

print "Analysis of the geometry box1 :"
subShellList = geompy.SubShapeAll(box1, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(box1, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(box1, geompy.ShapeType["EDGE"])

print "number of Shells in box1 : ", len(subShellList)
print "number of Faces  in box1 : ", len(subFaceList)
print "number of Edges  in box1 : ", len(subEdgeList)

box2 = geompy.MakeBox(100., 0., 0., 200., 200., 300.)

idbox2 = geompy.addToStudy(box2, "box2")

print "Analysis of the geometry box2 :"
subShellList = geompy.SubShapeAll(box2, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(box2, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(box2, geompy.ShapeType["EDGE"])

print "number of Shells in box2 : ", len(subShellList)
print "number of Faces  in box2 : ", len(subFaceList)
print "number of Edges  in box2 : ", len(subEdgeList)

# append the tow boxes to make ine shel, referrencing only once
# the internal interface

shell = geompy.MakePartition([box1, box2])
idshell = geompy.addToStudy(shell, "shell")

print "Analysis of the geometry shell (union of box1 and box2) :"
subShellList = geompy.SubShapeAll(shell, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(shell, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(shell, geompy.ShapeType["EDGE"])

print "number of Shells in shell : ", len(subShellList)
print "number of Faces  in shell : ", len(subFaceList)
print "number of Edges  in shell : ", len(subEdgeList)


### ---------------------------- SMESH --------------------------------------

# ---- init a Mesh with the shell

mesh = smesh.Mesh(shell, "MeshBox2")


# ---- set Hypothesis and Algorithm

print "-------------------------- NumberOfSegments"

numberOfSegments = 10

regular1D = mesh.Segment()
hypNbSeg = regular1D.NumberOfSegments(numberOfSegments)
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()
smesh.SetName(hypNbSeg, "NumberOfSegments_" + str(numberOfSegments))

print "-------------------------- MaxElementArea"

maxElementArea = 500

mefisto2D = mesh.Triangle()
hypArea = mefisto2D.MaxElementArea(maxElementArea)
print hypArea.GetName()
print hypArea.GetId()
print hypArea.GetMaxElementArea()
smesh.SetName(hypArea, "MaxElementArea_" + str(maxElementArea))

print "-------------------------- MaxElementVolume"

maxElementVolume = 500

netgen3D = mesh.Tetrahedron(smeshBuilder.NETGEN)
hypVolume = netgen3D.MaxElementVolume(maxElementVolume)
print hypVolume.GetName()
print hypVolume.GetId()
print hypVolume.GetMaxElementVolume()
smesh.SetName(hypVolume, "MaxElementVolume_" + str(maxElementVolume))

print "-------------------------- compute shell"
ret = mesh.Compute()
print ret
if ret != 0:
    log = mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the MeshBox2:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of triangles   : ", mesh.NbTriangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of tetrahedrons: ", mesh.NbTetras()
else:
    print "probleme when computing the mesh"

salome.sg.updateObjBrowser(True)
