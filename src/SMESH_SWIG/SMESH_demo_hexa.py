#
# Tetrahedrization of a geometry (box minus a inner cylinder).
# Hypothesis and algorithms for the mesh generation are global
#

import math
import salome
from salome import sg

import geompy

import SMESH
import smeshpy

# -----------------------------------------------------------------------------

geom = geompy.geom
myBuilder = geompy.myBuilder

ShapeTypeShell     = 3
ShapeTypeFace      = 4
ShapeTypeEdge      = 6

a = math.sqrt(2.)/4.
ma = - a
zero = 0.
un = 1.
mun= - un
demi = 1./2.

Orig = geom.MakePointStruct(zero,zero,zero)
P0 = geom.MakePointStruct(a,a,zero)
P1 = geom.MakePointStruct(zero,demi,zero)
P2 = geom.MakePointStruct(ma,a,zero)
P3 = geom.MakePointStruct(mun,un,zero)
P4 = geom.MakePointStruct(un,un,zero)
P5 = geom.MakePointStruct(zero,zero,un)

arc = geompy.MakeArc(P0,P1,P2)
e1 = geompy.MakeEdge(P2,P3)
e2 = geompy.MakeEdge(P3,P4)
e3 = geompy.MakeEdge(P4,P0)

list = []
list.append(arc._get_Name())
list.append(e1._get_Name())
list.append(e2._get_Name())
list.append(e3._get_Name())

wire = geompy.MakeWire(list)
face = geompy.MakeFace(wire,1)

dir = geompy.MakeVector(Orig,P5)
vol1 = geompy.MakePipe(dir,face)

angle = math.pi/2.
dir = geom.MakeAxisStruct(zero,zero,zero,zero,zero,un)
vol2 = geompy.MakeRotation(vol1,dir,angle)

vol3 = geompy.MakeRotation(vol2,dir,angle)

vol4 = geompy.MakeRotation(vol3,dir,angle)

list = []
list.append(vol1._get_Name())
list.append(vol2._get_Name())
list.append(vol3._get_Name())
list.append(vol4._get_Name())

volComp = geompy.MakeCompound(list)

tol3d = 1.e-3
vol = geom.MakeGlueFaces(volComp,tol3d)
idVol = geompy.addToStudy(vol,"volume")

print "Analysis of the final volume:"
subShellList=geompy.SubShapeAll(vol,ShapeTypeShell)
subFaceList=geompy.SubShapeAll(vol,ShapeTypeFace)
subEdgeList=geompy.SubShapeAll(vol,ShapeTypeEdge)

print "number of Shells in the volume : ",len(subShellList)
print "number of Faces in the volume : ",len(subFaceList)
print "number of Edges in the volume : ",len(subEdgeList)

# ---- launch SMESH

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

gen=smeshpy.smeshpy()

# ---- create Hypothesis

print "-------------------------- create Hypothesis"

print "-------------------------- NumberOfSegments"

numberOfSegments = 10

hypothesis=gen.CreateHypothesis("NumberOfSegments")
hypNbSeg=hypothesis._narrow(SMESH.SMESH_NumberOfSegments)
hypNbSeg.SetNumberOfSegments(numberOfSegments)
hypNbSegID = hypNbSeg.GetId()
print hypNbSeg.GetName()
print hypNbSegID
print hypNbSeg.GetNumberOfSegments()

idseg = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypNbSeg) )
smeshgui.SetName(idseg, "NumberOfSegments")

# ---- create Algorithms

print "-------------------------- create Algorithms"

print "-------------------------- Regular_1D"

hypothesis=gen.CreateHypothesis("Regular_1D")
regular1D = hypothesis._narrow(SMESH.SMESH_Regular_1D)
regularID = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(regular1D) )
smeshgui.SetName(regularID, "Wire Discretisation")

print "-------------------------- Quadrangle_2D"

hypothesis=gen.CreateHypothesis("Quadrangle_2D")
quad2D = hypothesis._narrow(SMESH.SMESH_Quadrangle_2D)
quadID = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(quad2D) )
smeshgui.SetName(quadID, "Quadrangle_2D")

print "-------------------------- Hexa_3D"

hypothesis=gen.CreateHypothesis("Hexa_3D")
hexa3D = hypothesis._narrow(SMESH.SMESH_Hexa_3D)
hexaID = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(hexa3D) )
smeshgui.SetName(hexaID, "Hexa_3D")

# ---- init a Mesh with the volume

mesh=gen.Init(idVol)
idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName(idmesh, "meshVolume")
smeshgui.SetShape(idVol, idmesh)

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

print "-------------------------- set algoritms"

smeshgui.SetAlgorithms( idmesh, regularID)
smeshgui.SetHypothesis( idmesh, idseg )
smeshgui.SetAlgorithms( idmesh, quadID )
smeshgui.SetAlgorithms( idmesh, hexaID )

sg.updateObjBrowser(1)

print "-------------------------- compute the mesh of the volume"
ret=gen.Compute(mesh,idVol)
print ret
if ret != 0:
    log=mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
else:
    print "problem when Computing the mesh"

sg.updateObjBrowser(1)
