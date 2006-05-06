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
# Tetrahedrization of a simple box. Hypothesis and algorithms for
# the mesh generation are global
#

import salome
import geompy

import StdMeshers
import NETGENPlugin

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);

# ---- define a boxe

box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)

idbox = geompy.addToStudy(box, "box")

print "Analysis of the geometry box :"
subShellList = geompy.SubShapeAll(box, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(box, geompy.ShapeType["EDGE"])

print "number of Shells in box : ", len(subShellList)
print "number of Faces  in box : ", len(subFaceList)
print "number of Edges  in box : ", len(subEdgeList)


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

# ---- init a Mesh with the boxe

mesh = smesh.CreateMesh(box)
smeshgui.SetName(salome.ObjectToID(mesh), "MeshBox")

# ---- add hypothesis to the boxe

print "-------------------------- add hypothesis to the box"

mesh.AddHypothesis(box,regular1D)
mesh.AddHypothesis(box,hypNbSeg)

mesh.AddHypothesis(box,mefisto2D)
mesh.AddHypothesis(box,hypArea)

mesh.AddHypothesis(box,netgen3D)
mesh.AddHypothesis(box,hypVolume)

salome.sg.updateObjBrowser(1)

print "-------------------------- compute the mesh of the boxe"
ret = smesh.Compute(mesh,box)
print ret
if ret != 0:
    log = mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the MeshBox:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of triangles   : ", mesh.NbTriangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of tetrahedrons: ", mesh.NbTetras()
else:
    print "probleme when computing the mesh"
