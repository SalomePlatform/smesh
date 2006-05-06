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
#
# Tetrahedrization of the geometry union of 3 boxes aligned where the middle
# one has a race in common with the two others.
# Hypothesis and algorithms for the mesh generation are global
#

import salome
import geompy

import StdMeshers
import NETGENPlugin

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);

# ---- define 3 boxes box1, box2 and box3

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

box3 = geompy.MakeBox(0., 0., 300., 200., 200., 500.)

idbox3 = geompy.addToStudy(box3, "box3")

print "Analysis of the geometry box3 :"
subShellList = geompy.SubShapeAll(box3, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(box3, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(box3, geompy.ShapeType["EDGE"])

print "number of Shells in box3 : ", len(subShellList)
print "number of Faces  in box3 : ", len(subFaceList)
print "number of Edges  in box3 : ", len(subEdgeList)

shell = geompy.MakePartition([box1, box2, box3])
idshell = geompy.addToStudy(shell,"shell")

print "Analysis of the geometry shell (union of box1, box2 and box3) :"
subShellList = geompy.SubShapeAll(shell, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(shell, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(shell, geompy.ShapeType["EDGE"])

print "number of Shells in shell : ", len(subShellList)
print "number of Faces  in shell : ", len(subFaceList)
print "number of Edges  in shell : ", len(subEdgeList)


### ---------------------------- SMESH --------------------------------------

# ---- create Hypothesis

print "-------------------------- create Hypothesis"

print "-------------------------- NumberOfSegments"

numberOfSegments = 10

hypNbSeg = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg.SetNumberOfSegments(numberOfSegments)

print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()

smeshgui.SetName(salome.ObjectToID(hypNbSeg), "NumberOfSegments_10")

print "-------------------------- MaxElementArea"

maxElementArea = 500

hypArea = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hypArea.SetMaxElementArea(maxElementArea)

print hypArea.GetName()
print hypArea.GetId()
print hypArea.GetMaxElementArea()

smeshgui.SetName(salome.ObjectToID(hypArea), "MaxElementArea_500")

print "-------------------------- MaxElementVolume"

maxElementVolume = 500

hypVolume = smesh.CreateHypothesis("MaxElementVolume", "libStdMeshersEngine.so")
hypVolume.SetMaxElementVolume(maxElementVolume)

print hypVolume.GetName()
print hypVolume.GetId()
print hypVolume.GetMaxElementVolume()

smeshgui.SetName(salome.ObjectToID(hypVolume), "MaxElementVolume_500")

# ---- create Algorithms

print "-------------------------- create Algorithms"

print "-------------------------- Regular_1D"

regular1D = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
smeshgui.SetName(salome.ObjectToID(regular1D), "Wire Discretisation")

print "-------------------------- MEFISTO_2D"

mefisto2D = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")
smeshgui.SetName(salome.ObjectToID(mefisto2D), "MEFISTO_2D")

print "-------------------------- NETGEN_3D"

netgen3D = smesh.CreateHypothesis("NETGEN_3D", "libNETGENEngine.so")
smeshgui.SetName(salome.ObjectToID(netgen3D), "NETGEN_3D")

# ---- init a Mesh with the shell

mesh = smesh.CreateMesh(shell)
smeshgui.SetName(salome.ObjectToID(mesh), "MeshBox3")

# ---- add hypothesis to shell

print "-------------------------- add hypothesis to shell"

mesh.AddHypothesis(shell,regular1D)
mesh.AddHypothesis(shell,hypNbSeg)

mesh.AddHypothesis(shell,mefisto2D)
mesh.AddHypothesis(shell,hypArea)

mesh.AddHypothesis(shell,netgen3D)
mesh.AddHypothesis(shell,hypVolume)

salome.sg.updateObjBrowser(1)

print "-------------------------- compute shell"
ret = smesh.Compute(mesh,shell)
print ret
if ret != 0:
    log = mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the MeshBox3:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of triangles   : ", mesh.NbTriangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of tetrahedrons: ", mesh.NbTetras()
else:
    print "probleme when computing the mesh"
