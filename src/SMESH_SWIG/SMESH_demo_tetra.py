#
# Tetrahedrization of a geometry (box minus a inner cylinder).
# Hypothesis and algorithms for the mesh generation are global
#

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

# ---- define a boxe and a cylinder
x0 = -1.
y0 = -1.
z0 = -1.

x1 = 1.
y1 = 1.
z1 = 1.

P0 = geom.MakePointStruct(0.,-1.,0.)
P1 = geom.MakePointStruct(0.,1.,0.)
Vect = geom.MakeDirection(P1)
radius = 0.5
height = 2.

boxe = geompy.MakeBox(x0,y0,z0,x1,y1,z1)

cylinder = geompy.MakeCylinder(P0,Vect,radius,height)

shape = geompy.MakeBoolean(boxe,cylinder,2)
idshape = geompy.addToStudy(shape,"shape")

print ""

print "Analysis of the shape :"
subShellList=geompy.SubShapeAll(shape,ShapeTypeShell)
subFaceList=geompy.SubShapeAll(shape,ShapeTypeFace)
subEdgeList=geompy.SubShapeAll(shape,ShapeTypeEdge)

print "number of Shells in the shape : ",len(subShellList)
print "number of Faces in the shape : ",len(subFaceList)
print "number of Edges in the shape : ",len(subEdgeList)

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

print "-------------------------- LengthFromEdges"

hypothesis=gen.CreateHypothesis("LengthFromEdges")
hypLengthFromEdge=hypothesis._narrow(SMESH.SMESH_LengthFromEdges)
hypLengthFromEdgeID = hypLengthFromEdge.GetId()
print hypLengthFromEdge.GetName()
print hypLengthFromEdgeID

idlenfromedge = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypLengthFromEdge) )
smeshgui.SetName(idlenfromedge, "LengthFromEdge")

print "-------------------------- MaxElementVolume"

maxElementVolume = 0.5

hypothesis=gen.CreateHypothesis("MaxElementVolume")
hypVolume=hypothesis._narrow(SMESH.SMESH_MaxElementVolume)
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

mesh=gen.Init(idshape)
idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName(idmesh, "MeshShape")
smeshgui.SetShape(idshape, idmesh)

# ---- add hypothesis to flight

print "-------------------------- add hypothesis to the shape"

ret=mesh.AddHypothesis(shape,regular1D)
print ret
ret=mesh.AddHypothesis(shape,hypNbSeg)
print ret
ret=mesh.AddHypothesis(shape,mefisto2D)
print ret
ret=mesh.AddHypothesis(shape,hypLengthFromEdge)
print ret
ret=mesh.AddHypothesis(shape,netgen3D)
print ret
ret=mesh.AddHypothesis(shape,hypVolume)
print ret

smeshgui.SetAlgorithms( idmesh, regularID)
smeshgui.SetHypothesis( idmesh, idseg )
smeshgui.SetAlgorithms( idmesh, mefistoID )
smeshgui.SetHypothesis( idmesh, idlenfromedge)
smeshgui.SetAlgorithms( idmesh, netgenID )
smeshgui.SetHypothesis( idmesh, idvolume )

sg.updateObjBrowser(1)


print "-------------------------- compute the mesh of the shape"
ret=gen.Compute(mesh,idshape)
print ret
if ret != 0:
    log=mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
else:
    print "probleme when computing the mesh"

sg.updateObjBrowser(1)
