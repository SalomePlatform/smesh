#
# Tetrahedrization of a simple box. Hypothesis and algorithms for
# the mesh generation are global
#

import salome
from salome import sg

import geompy

import SMESH
import smeshpy

# -----------------------------------------------------------------------------

##geom = salome.lcc.FindOrLoadComponent("FactoryServer", "Geometry")
##myBuilder = salome.myStudy.NewBuilder()

geom = geompy.geom
myBuilder = geompy.myBuilder

ShapeTypeShell     = 3
ShapeTypeFace      = 4
ShapeTypeEdge      = 6

# ---- define a boxe

box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)

idbox = geompy.addToStudy(box,"box")

print "Analysis of the geometry box :"
subShellList=geompy.SubShapeAll(box,ShapeTypeShell)
subFaceList=geompy.SubShapeAll(box,ShapeTypeFace)
subEdgeList=geompy.SubShapeAll(box,ShapeTypeEdge)

print "number of Shells in box : ",len(subShellList)
print "number of Faces in box : ",len(subFaceList)
print "number of Edges in box : ",len(subEdgeList)

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

# ---- init a Mesh with the boxe

mesh=gen.Init(idbox)
idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName(idmesh, "MeshBox")
smeshgui.SetShape(idbox, idmesh)

# ---- add hypothesis to the boxe

print "-------------------------- add hypothesis to the boxe"

ret=mesh.AddHypothesis(box,regular1D)
print ret
ret=mesh.AddHypothesis(box,hypNbSeg)
print ret
ret=mesh.AddHypothesis(box,mefisto2D)
print ret
ret=mesh.AddHypothesis(box,hypArea)
print ret
ret=mesh.AddHypothesis(box,netgen3D)
print ret
ret=mesh.AddHypothesis(box,hypVolume)
print ret

smeshgui.SetAlgorithms( idmesh, regularID)
smeshgui.SetHypothesis( idmesh, idseg )
smeshgui.SetAlgorithms( idmesh, mefistoID )
smeshgui.SetHypothesis( idmesh, idarea )
smeshgui.SetAlgorithms( idmesh, netgenID )
smeshgui.SetHypothesis( idmesh, idvolume )

sg.updateObjBrowser(1)


print "-------------------------- compute the mesh of the boxe"
ret=gen.Compute(mesh,idbox)
print ret
log=mesh.GetLog(0) # no erase trace
for linelog in log:
    print linelog


sg.updateObjBrowser(1)
