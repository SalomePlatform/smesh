#  Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/
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
import geompy

import StdMeshers
import NETGENPlugin

geom  = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);

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

# ---- create Hypothesis

print "-------------------------- create Hypothesis"

print "-------------------------- NumberOfSegments the global one"

numberOfSegments = 10

hypNbSeg=smesh.CreateHypothesis("NumberOfSegments","libStdMeshersEngine.so")
hypNbSeg.SetNumberOfSegments(numberOfSegments)
hypNbSegID = hypNbSeg.GetId()
print hypNbSeg.GetName()
print hypNbSegID
print hypNbSeg.GetNumberOfSegments()

smeshgui.SetName(salome.ObjectToID(hypNbSeg), "NumberOfSegments")

print "-------------------------- NumberOfSegments in the Z direction"

numberOfSegmentsZ = 40

hypNbSegZ=smesh.CreateHypothesis("NumberOfSegments","libStdMeshersEngine.so")
hypNbSegZ.SetNumberOfSegments(numberOfSegmentsZ)
hypNbSegZID = hypNbSegZ.GetId()
print hypNbSegZ.GetName()
print hypNbSegZID
print hypNbSegZ.GetNumberOfSegments()

smeshgui.SetName(salome.ObjectToID(hypNbSegZ), "NumberOfSegmentsZ")

# ---- create Algorithms

print "-------------------------- create Algorithms"

print "-------------------------- Regular_1D"

regular1D=smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
smeshgui.SetName(salome.ObjectToID(regular1D), "Wire Discretisation")

print "-------------------------- Quadrangle_2D"

quad2D=smesh.CreateHypothesis("Quadrangle_2D", "libStdMeshersEngine.so")
smeshgui.SetName(salome.ObjectToID(quad2D), "Quadrangle_2D")

print "-------------------------- Hexa_3D"

hexa3D=smesh.CreateHypothesis("Hexa_3D", "libStdMeshersEngine.so")
smeshgui.SetName(salome.ObjectToID(hexa3D), "Hexa_3D")

# ---- init a Mesh with the volume

mesh = smesh.CreateMesh(vol)
smeshgui.SetName(salome.ObjectToID(mesh), "meshVolume")

# ---- add hypothesis to the volume

print "-------------------------- add hypothesis to the volume"

ret=mesh.AddHypothesis(vol,regular1D)
print ret
ret=mesh.AddHypothesis(vol,hypNbSeg)
print ret
ret=mesh.AddHypothesis(vol,quad2D)
print ret
ret=mesh.AddHypothesis(vol,hexa3D)
print ret

for i in range(8):
    print "-------------------------- add hypothesis to edge in the Z directions", (i+1)

    subMeshEdgeZ = mesh.GetSubMesh(edgeZ[i],"SubMeshEdgeZ_"+str(i+1))

    retZ = mesh.AddHypothesis(edgeZ[i],hypNbSegZ)
    print " add hyp Z ", retZ

salome.sg.updateObjBrowser(1)

print "-------------------------- compute the mesh of the volume"

ret=smesh.Compute(mesh,vol)

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

salome.sg.updateObjBrowser(1)
