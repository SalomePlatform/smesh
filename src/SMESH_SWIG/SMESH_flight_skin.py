#
# Triangulation of the skin of the geometry from a Brep representing a plane
# This geometry is from EADS
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

import os

# import a BRep
#before running this script, please be sure about
#the path the file fileName

filePath=os.environ["SMESH_ROOT_DIR"]
filePath=filePath+"/share/salome/resources/"

filename = "flight_solid.brep"
filename = filePath + filename

shape = geompy.ImportBREP(filename)
idShape = geompy.addToStudy(shape,"flight")

print "Analysis of the geometry flight :"
subShellList=geompy.SubShapeAll(shape,ShapeTypeShell)
subFaceList=geompy.SubShapeAll(shape,ShapeTypeFace)
subEdgeList=geompy.SubShapeAll(shape,ShapeTypeEdge)

print "number of Shells in flight : ",len(subShellList)
print "number of Faces in flight : ",len(subFaceList)
print "number of Edges in flight : ",len(subEdgeList)

# ---- launch SMESH

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

gen=smeshpy.smeshpy()

# ---- create Hypothesis

print "-------------------------- create Hypothesis"

print "-------------------------- LocalLength"

lengthOfSegments = 0.3

hypothesis=gen.CreateHypothesis("LocalLength")
hypLength=hypothesis._narrow(SMESH.SMESH_LocalLength)
hypLength.SetLength(lengthOfSegments)
hypLengthID = hypLength.GetId()
print hypLength.GetName()
print hypLengthID
print hypLength.GetLength()

idlen = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypLength) )
smeshgui.SetName(idlen, "LocalLength")

print "-------------------------- LengthFromEdges"

hypothesis=gen.CreateHypothesis("LengthFromEdges")
hypLengthFromEdge=hypothesis._narrow(SMESH.SMESH_LengthFromEdges)
hypLengthFromEdgeID = hypLengthFromEdge.GetId()
print hypLengthFromEdge.GetName()
print hypLengthFromEdgeID

idlenfromedge = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypLengthFromEdge) )
smeshgui.SetName(idlenfromedge, "LengthFromEdge")

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

mesh=gen.Init(idShape)
idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName(idmesh, "MeshFlight")
smeshgui.SetShape(idShape, idmesh)

# ---- add hypothesis to flight

print "-------------------------- add hypothesis to flight"

ret=mesh.AddHypothesis(shape,regular1D)
print ret
ret=mesh.AddHypothesis(shape,hypLength)
print ret
ret=mesh.AddHypothesis(shape,mefisto2D)
print ret
ret=mesh.AddHypothesis(shape,hypLengthFromEdge)
print ret

smeshgui.SetAlgorithms( idmesh, regularID)
smeshgui.SetHypothesis( idmesh, idlen )
smeshgui.SetAlgorithms( idmesh, mefistoID )
smeshgui.SetHypothesis( idmesh, idlenfromedge)

sg.updateObjBrowser(1)


print "-------------------------- compute the skin flight"
ret=gen.Compute(mesh,idShape)
print ret
if ret != 0:
    log=mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
else:
    print "probleme when computing the mesh"

sg.updateObjBrowser(1)
