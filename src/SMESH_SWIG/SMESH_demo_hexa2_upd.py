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

#==============================================================================
#  Info.
#  Bug (from script, bug)   : SMESH_demo_hexa2_upd.py, PAL6781
#  Modified                 : 25/11/2004
#  Author                   : Kovaltchuk Alexey
#  Project                  : PAL/SALOME
#==============================================================================
# Tetrahedrization of a geometry (box minus a inner cylinder).
# Hypothesis and algorithms for the mesh generation are not global:
# the mesh of some edges is thinner
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


# -----------------------------------------------------------------------------

ShapeTypeShell     = 3
ShapeTypeFace      = 4
ShapeTypeEdge      = 6

a = math.sqrt(2.)/4.
ma = - a
zero = 0.
un = 1.
mun= - un
demi = 1./2.

Orig = geompy.MakeVertex(zero,zero,zero)
P0 = geompy.MakeVertex(a,a,zero)
P1 = geompy.MakeVertex(zero,demi,zero)
P2 = geompy.MakeVertex(ma,a,zero)
P3 = geompy.MakeVertex(mun,un,zero)
P4 = geompy.MakeVertex(un,un,zero)
P5 = geompy.MakeVertex(zero,zero,un)

arc = geompy.MakeArc(P0,P1,P2)
e1 = geompy.MakeEdge(P2,P3)
e2 = geompy.MakeEdge(P3,P4)
e3 = geompy.MakeEdge(P4,P0)

list = []
list.append(arc)
list.append(e1)
list.append(e2)
list.append(e3)

wire = geompy.MakeWire(list)
face = geompy.MakeFace(wire,1)

dir = geompy.MakeVector(Orig,P5)
vol1 = geompy.MakePipe(face,dir)

angle = math.pi/2.
#dir = geom.MakeVector(Orig,P5)
vol2 = geompy.MakeRotation(vol1,dir,angle)

vol3 = geompy.MakeRotation(vol2,dir,angle)

vol4 = geompy.MakeRotation(vol3,dir,angle)

list = []
list.append(vol1)
list.append(vol2)
list.append(vol3)
list.append(vol4)

volComp = geompy.MakeCompound(list)

tol3d = 1.e-3
vol = geompy.MakeGlueFaces(volComp,tol3d)
idVol = geompy.addToStudy(vol,"volume")

print "Analysis of the final volume:"
subShellList = geompy.SubShapeAllSorted(vol,ShapeTypeShell)
subFaceList = geompy.SubShapeAllSorted(vol,ShapeTypeFace)
subEdgeList = geompy.SubShapeAllSorted(vol,ShapeTypeEdge)

print "number of Shells in the volume : ",len(subShellList)
print "number of Faces in the volume : ",len(subFaceList)
print "number of Edges in the volume : ",len(subEdgeList)

idSubEdge = []
for k in range(len(subEdgeList)):
    idSubEdge.append(geompy.addToStudyInFather(vol,subEdgeList[k],"SubEdge"+str(k)))

edgeZ = []
edgeZ.append(subEdgeList[0])
edgeZ.append(subEdgeList[3])
edgeZ.append(subEdgeList[10])
edgeZ.append(subEdgeList[11])
edgeZ.append(subEdgeList[20])
edgeZ.append(subEdgeList[21])
edgeZ.append(subEdgeList[28])
edgeZ.append(subEdgeList[31])

idEdgeZ = []
for i in range(8):
    idEdgeZ.append(geompy.addToStudyInFather(vol,edgeZ[i],"EdgeZ"+str(i+1)))

### ---------------------------- SMESH --------------------------------------
smesh.SetCurrentStudy(salome.myStudy)

# ---- init a Mesh with the volume

mesh = smesh.Mesh(vol, "meshVolume")

# ---- set Hypothesis and Algorithm to main shape

print "-------------------------- NumberOfSegments the global one"

numberOfSegments = 10

regular1D = mesh.Segment()
regular1D.SetName("Wire Discretisation")
hypNbSeg = regular1D.NumberOfSegments(numberOfSegments)
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()
smesh.SetName(hypNbSeg, "NumberOfSegments")


print "-------------------------- Quadrangle_2D"

quad2D=mesh.Quadrangle()
quad2D.SetName("Quadrangle_2D")

print "-------------------------- Hexa_3D"

hexa3D=mesh.Hexahedron()
hexa3D.SetName("Hexa_3D")


print "-------------------------- NumberOfSegments in the Z direction"

numberOfSegmentsZ = 40

for i in range(8):
    print "-------------------------- add hypothesis to edge in the Z directions", (i+1)

    algo = mesh.Segment(edgeZ[i])
    hyp = algo.NumberOfSegments(numberOfSegmentsZ)
    smesh.SetName(hyp, "NumberOfSegmentsZ")
    smesh.SetName(algo.GetSubMesh(), "SubMeshEdgeZ_"+str(i+1))
  

salome.sg.updateObjBrowser(True)

print "-------------------------- compute the mesh of the volume"

ret=mesh.Compute()

print ret
if ret != 0:
##    log=mesh.GetLog(0) # no erase trace
##    for linelog in log:
##        print linelog
    print "Information about the MeshBox :"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of triangles   : ", mesh.NbTriangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of tetrahedrons: ", mesh.NbTetras()
else:
    print "problem when Computing the mesh"

salome.sg.updateObjBrowser(True)
