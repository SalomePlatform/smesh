#
# Tetrahedrization of the geometry union of 2 boxes having a face in common
# Hypothesis and algorithms for the mesh generation are global
#

import salome
from salome import sg

import geompy

import SMESH
import smeshpy

geom = geompy.geom
myBuilder = geompy.myBuilder

ShapeTypeShell     = 3
ShapeTypeFace      = 4
ShapeTypeEdge      = 6

# ---- define 2 boxes box1 and box2

box1 = geompy.MakeBox(0., 0., 0., 100., 200., 300.)

idbox1 = geompy.addToStudy(box1,"box1")

print "Analysis of the geometry box1 :"
subShellList=geompy.SubShapeAll(box1,ShapeTypeShell)
subFaceList=geompy.SubShapeAll(box1,ShapeTypeFace)
subEdgeList=geompy.SubShapeAll(box1,ShapeTypeEdge)

print "number of Shells in box1 : ",len(subShellList)
print "number of Faces in box1 : ",len(subFaceList)
print "number of Edges in box1 : ",len(subEdgeList)

box2 = geompy.MakeBox(100., 0., 0., 200., 200., 300.)

idbox2 = geompy.addToStudy(box2,"box2")

print "Analysis of the geometry box2 :"
subShellList=geompy.SubShapeAll(box2,ShapeTypeShell)
subFaceList=geompy.SubShapeAll(box2,ShapeTypeFace)
subEdgeList=geompy.SubShapeAll(box2,ShapeTypeEdge)

print "number of Shells in box2 : ",len(subShellList)
print "number of Faces in box2 : ",len(subFaceList)
print "number of Edges in box2 : ",len(subEdgeList)

blocs = []
blocs.append(box1._get_Name())
blocs.append(box2._get_Name())

# append the tow boxes to make ine shel, referrencing only once
# the internal interface

shell = geompy.Partition(blocs)
idshell = geompy.addToStudy(shell,"shell")

print "Analysis of the geometry shell (union of box1 and box2) :"
subShellList=geompy.SubShapeAll(shell,ShapeTypeShell)
subFaceList=geompy.SubShapeAll(shell,ShapeTypeFace)
subEdgeList=geompy.SubShapeAll(shell,ShapeTypeEdge)

print "number of Shells in shell : ",len(subShellList)
print "number of Faces in shell : ",len(subFaceList)
print "number of Edges in shell : ",len(subEdgeList)

# ---- launch SMESH

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

gen=smeshpy.smeshpy()

# ---- create Hypothesis

print "-------------------------- create Hypothesis"

print "-------------------------- NumberOfSegments"

numberOfSegments = 10

hyp1=gen.CreateHypothesis("NumberOfSegments")
hypNbSeg=hyp1._narrow(SMESH.SMESH_NumberOfSegments)
hypNbSeg.SetNumberOfSegments(numberOfSegments)
hypNbSegID = hypNbSeg.GetId()
print hypNbSeg.GetName()
print hypNbSegID
print hypNbSeg.GetNumberOfSegments()

idseg = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypNbSeg) )
smeshgui.SetName(idseg, "NumberOfSegments")

print "-------------------------- MaxElementArea"

maxElementArea = 500

hyp2=gen.CreateHypothesis("MaxElementArea")
hypArea=hyp2._narrow(SMESH.SMESH_MaxElementArea)
hypArea.SetMaxElementArea(maxElementArea)
print hypArea.GetName()
print hypArea.GetId()
print hypArea.GetMaxElementArea()

idarea = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypArea) )
smeshgui.SetName(idarea, "MaxElementArea")

print "-------------------------- MaxElementVolume"

maxElementVolume = 500

hyp3=gen.CreateHypothesis("MaxElementVolume")
hypVolume=hyp3._narrow(SMESH.SMESH_MaxElementVolume)
hypVolume.SetMaxElementVolume(maxElementVolume)
print hypVolume.GetName()
print hypVolume.GetId()
print hypVolume.GetMaxElementVolume()

idvolume = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypVolume) )
smeshgui.SetName(idvolume, "MaxElementVolume")

# ---- create Algorithms

print "-------------------------- create Algorithms"

print "-------------------------- Regular_1D"

hypothesis=gen.CreateHypothesis("Regular_1D")
regular1D = hypothesis._narrow(SMESH.SMESH_Regular_1D)
regularID = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(regular1D) )
smeshgui.SetName(regularID, "Wire Discretisation")

print "-------------------------- MEFISTO_2D"

hypothesis=gen.CreateHypothesis("MEFISTO_2D")
mefisto2D = hypothesis._narrow(SMESH.SMESH_MEFISTO_2D)
mefistoID = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(mefisto2D) )
smeshgui.SetName(mefistoID, "MEFISTO_2D")

print "-------------------------- NETGEN_3D"

hypothesis=gen.CreateHypothesis("NETGEN_3D")
netgen3D = hypothesis._narrow(SMESH.SMESH_NETGEN_3D)
netgenID = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(netgen3D) )
smeshgui.SetName(netgenID, "NETGEN_3D")

# ---- init a Mesh with the shell

mesh=gen.Init(idshell)
idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName(idmesh, "MeshBox2")
smeshgui.SetShape(idshell, idmesh)

# ---- add hypothesis to shell

print "-------------------------- add hypothesis to shell"

ret=mesh.AddHypothesis(shell,regular1D)
print ret
ret=mesh.AddHypothesis(shell,hypNbSeg)
print ret
ret=mesh.AddHypothesis(shell,mefisto2D)
print ret
ret=mesh.AddHypothesis(shell,hypArea)
print ret
ret=mesh.AddHypothesis(shell,netgen3D)
print ret
ret=mesh.AddHypothesis(shell,hypVolume)
print ret

smeshgui.SetAlgorithms( idmesh, regularID)
smeshgui.SetHypothesis( idmesh, idseg )
smeshgui.SetAlgorithms( idmesh, mefistoID )
smeshgui.SetHypothesis( idmesh, idarea )
smeshgui.SetAlgorithms( idmesh, netgenID )
smeshgui.SetHypothesis( idmesh, idvolume )

sg.updateObjBrowser(1)


print "-------------------------- compute shell"
ret=gen.Compute(mesh,idshell)
print ret
log=mesh.GetLog(0) # no erase trace
for linelog in log:
    print linelog


sg.updateObjBrowser(1)
