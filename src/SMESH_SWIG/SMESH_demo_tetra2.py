#
# Tetrahedrization of a geometry (box minus a inner cylinder).
# Hypothesis and algorithms for the mesh generation are not
# global: the mesh of some edges is thinner.
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

print "Analysis of the shape :"
subShellList=geompy.SubShapeAllSorted(shape,ShapeTypeShell)
subFaceList=geompy.SubShapeAllSorted(shape,ShapeTypeFace)
subEdgeList=geompy.SubShapeAllSorted(shape,ShapeTypeEdge)

print "number of Shells in the shape : ",len(subShellList)
print "number of Faces in the shape : ",len(subFaceList)
print "number of Edges in the shape : ",len(subEdgeList)

idSubEdge = []
for k in range(len(subEdgeList)):
    idSubEdge.append(geompy.addToStudyInFather(shape,subEdgeList[k],"SubEdge"+str(k)))

circle1 = subEdgeList[5]
idCircle1 = geompy.addToStudyInFather(shape,circle1,"circle1")

circle2 = subEdgeList[9]
idCircle2 = geompy.addToStudyInFather(shape,circle2,"circle2")

height = subEdgeList[7]
idHeight = geompy.addToStudyInFather(shape,height,"height")

# ---- launch SMESH

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

gen=smeshpy.smeshpy()

# ---- create Hypothesis

print "-------------------------- create Hypothesis"

print "-------------------------- NumberOfSegments Edge of the boxe"

numberOfSegmentsBoxe = 10

hypothesis=gen.CreateHypothesis("NumberOfSegments")
hypNbSegBoxe=hypothesis._narrow(SMESH.SMESH_NumberOfSegments)
hypNbSegBoxe.SetNumberOfSegments(numberOfSegmentsBoxe)
hypNbSegID = hypNbSegBoxe.GetId()
print hypNbSegBoxe.GetName()
print hypNbSegID
print hypNbSegBoxe.GetNumberOfSegments()

idSegBoxe = smeshgui.AddNewHypothesis(salome.orb.object_to_string(hypNbSegBoxe))
smeshgui.SetName(idSegBoxe, "NumberOfSegmentsBoxe")

print "-------------------------- NumberOfSegments Edge of the cylinder"

numberOfSegmentsCylinder = 40

hypothesis=gen.CreateHypothesis("NumberOfSegments")
hypNbSegCylinder=hypothesis._narrow(SMESH.SMESH_NumberOfSegments)
hypNbSegCylinder.SetNumberOfSegments(numberOfSegmentsCylinder)
hypNbSegID = hypNbSegCylinder.GetId()
print hypNbSegCylinder.GetName()
print hypNbSegID
print hypNbSegCylinder.GetNumberOfSegments()

idSegCylinder = smeshgui.AddNewHypothesis(salome.orb.object_to_string(hypNbSegCylinder))
smeshgui.SetName(idSegCylinder, "NumberOfSegmentsCylinder")

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
ret=mesh.AddHypothesis(shape,hypNbSegBoxe)
print ret
ret=mesh.AddHypothesis(shape,mefisto2D)
print ret
ret=mesh.AddHypothesis(shape,hypLengthFromEdge)
print ret
ret=mesh.AddHypothesis(shape,netgen3D)
print ret
ret=mesh.AddHypothesis(shape,hypVolume)
print ret

print "-------------------------- set algoritms"

smeshgui.SetAlgorithms( idmesh, regularID)
smeshgui.SetHypothesis( idmesh, idSegBoxe )
smeshgui.SetAlgorithms( idmesh, mefistoID )
smeshgui.SetHypothesis( idmesh, idlenfromedge)
smeshgui.SetAlgorithms( idmesh, netgenID )
smeshgui.SetHypothesis( idmesh, idvolume )

print "-------------------------- add hypothesis to the first circle"

subMeshEdgeCircle1 = mesh.GetElementsOnShape(circle1)
retCircle1 = mesh.AddHypothesis(circle1,hypNbSegCylinder)
print " add hyp to C1 ", retCircle1

idsmCircle1 = smeshgui.AddSubMeshOnShape(
    idmesh,idCircle1,salome.orb.object_to_string(subMeshEdgeCircle1),
    ShapeTypeEdge)

smeshgui.SetName(idsmCircle1, "SubMeshEdgeCircle1")
smeshgui.SetHypothesis(idsmCircle1, idSegCylinder)

print "-------------------------- add hypothesis to the second circle"

subMeshEdgeCircle2 = mesh.GetElementsOnShape(circle2)
retCircle2 = mesh.AddHypothesis(circle2,hypNbSegCylinder)
print " add hyp to C2 ", retCircle2

idsmCircle2 = smeshgui.AddSubMeshOnShape(
    idmesh,idCircle2,salome.orb.object_to_string(subMeshEdgeCircle2),
    ShapeTypeEdge)

smeshgui.SetName(idsmCircle2, "SubMeshEdgeCircle2")
smeshgui.SetHypothesis(idsmCircle2, idSegCylinder)

print "-------------------------- add hypothesis to the height of the cylinder"

subMeshEdgeHeight = mesh.GetElementsOnShape(height)
retHeight = mesh.AddHypothesis(height,hypNbSegCylinder)
print " add hyp to H ", retHeight

idsmHeight = smeshgui.AddSubMeshOnShape(
    idmesh,idHeight,salome.orb.object_to_string(subMeshEdgeHeight),
    ShapeTypeEdge)

smeshgui.SetName(idsmHeight, "SubMeshEdgeHeight")
smeshgui.SetHypothesis(idsmHeight, idSegCylinder)

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
