#
# Hexahedrization of a simple box. Hypothesis and algorithms for
# the mesh generation are not global: the mesh of some edges is thinner
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

# ---- define a boxe

box = geompy.MakeBox(0., 0., 0., 1., 1., 1.)

idbox = geompy.addToStudy(box,"box")

print "Analysis of the geometry box :"
subShellList=geompy.SubShapeAllSorted(box,ShapeTypeShell)
subFaceList=geompy.SubShapeAllSorted(box,ShapeTypeFace)
subEdgeList=geompy.SubShapeAllSorted(box,ShapeTypeEdge)

print "number of Shells in box : ",len(subShellList)
print "number of Faces in box : ",len(subFaceList)
print "number of Edges in box : ",len(subEdgeList)

idSubEdge = []
for k in range(len(subEdgeList)):
    idSubEdge.append(geompy.addToStudyInFather(box,subEdgeList[k],"SubEdge"+str(k)))

edgeX = []
edgeX.append(subEdgeList[4])
edgeX.append(subEdgeList[5])
edgeX.append(subEdgeList[6])
edgeX.append(subEdgeList[7])

edgeY = []
edgeY.append(subEdgeList[1])
edgeY.append(subEdgeList[2])
edgeY.append(subEdgeList[9])
edgeY.append(subEdgeList[10])

edgeZ = []
edgeZ.append(subEdgeList[0])
edgeZ.append(subEdgeList[3])
edgeZ.append(subEdgeList[8])
edgeZ.append(subEdgeList[11])

idEdgeX = []
idEdgeY = []
idEdgeZ = []
for i in range(4):
    idEdgeX.append(geompy.addToStudyInFather(box,edgeX[i],"EdgeX"+str(i+1)))
    idEdgeY.append(geompy.addToStudyInFather(box,edgeY[i],"EdgeY"+str(i+1)))
    idEdgeZ.append(geompy.addToStudyInFather(box,edgeZ[i],"EdgeZ"+str(i+1)))

# ---- launch SMESH

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

gen=smeshpy.smeshpy()

# ---- create Hypothesis

print "-------------------------- create Hypothesis"

print "-------------------------- NumberOfSegments in X, Y, Z direction"

numberOfSegmentsX = 10

hyp1=gen.CreateHypothesis("NumberOfSegments")
hypNbSegX=hyp1._narrow(SMESH.SMESH_NumberOfSegments)
hypNbSegX.SetNumberOfSegments(numberOfSegmentsX)
hypNbSegXID = hypNbSegX.GetId()
print hypNbSegX.GetName()
print hypNbSegXID
print hypNbSegX.GetNumberOfSegments()

idsegX = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypNbSegX) )
smeshgui.SetName(idsegX, "NumberOfSegmentsX")

print ""

numberOfSegmentsY = 20

hyp1=gen.CreateHypothesis("NumberOfSegments")
hypNbSegY=hyp1._narrow(SMESH.SMESH_NumberOfSegments)
hypNbSegY.SetNumberOfSegments(numberOfSegmentsY)
hypNbSegYID = hypNbSegY.GetId()
print hypNbSegY.GetName()
print hypNbSegYID
print hypNbSegY.GetNumberOfSegments()

idsegY = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypNbSegY) )
smeshgui.SetName(idsegY, "NumberOfSegmentsY")

print ""

numberOfSegmentsZ = 40

hyp1=gen.CreateHypothesis("NumberOfSegments")
hypNbSegZ=hyp1._narrow(SMESH.SMESH_NumberOfSegments)
hypNbSegZ.SetNumberOfSegments(numberOfSegmentsZ)
hypNbSegZID = hypNbSegZ.GetId()
print hypNbSegZ.GetName()
print hypNbSegZID
print hypNbSegZ.GetNumberOfSegments()

idsegZ = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypNbSegZ) )
smeshgui.SetName(idsegZ, "NumberOfSegmentsZ")

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

# ---- init a Mesh with the boxe

mesh=gen.Init(idbox)
idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName(idmesh, "MeshBox")
smeshgui.SetShape(idbox, idmesh)

# ---- add hypothesis to the boxe

print "-------------------------- add hypothesis to the boxe"
print "                           the number of segments is globally set to"
print "                           NumberOfSegmentsX = ", numberOfSegmentsX

ret=mesh.AddHypothesis(box,regular1D)
print ret
ret=mesh.AddHypothesis(box,hypNbSegX)
print ret
ret=mesh.AddHypothesis(box,quad2D)
print ret
ret=mesh.AddHypothesis(box,hexa3D)
print ret

print "-------------------------- set algoritms"

smeshgui.SetAlgorithms( idmesh, regularID)
smeshgui.SetHypothesis( idmesh, idsegX )
smeshgui.SetAlgorithms( idmesh, quadID )
smeshgui.SetAlgorithms( idmesh, hexaID )

for i in range(4):
    print "-------------------------- add hypothesis to edge in the Y and Z directions", (i+1)

    subMeshEdgeY = mesh.GetElementsOnShape(edgeY[i])
    subMeshEdgeZ = mesh.GetElementsOnShape(edgeZ[i])

    retY = mesh.AddHypothesis(edgeY[i],hypNbSegY)
    retZ = mesh.AddHypothesis(edgeZ[i],hypNbSegZ)
    print " add hyp Y ", retY, " Z ", retZ

    idsmY = smeshgui.AddSubMeshOnShape(
        idmesh,idEdgeY[i],salome.orb.object_to_string(subMeshEdgeY),
        ShapeTypeEdge)
    idsmZ = smeshgui.AddSubMeshOnShape(
        idmesh,idEdgeZ[i],salome.orb.object_to_string(subMeshEdgeZ),
        ShapeTypeEdge)

    smeshgui.SetName(idsmY, "SubMeshEdgeY_"+str(i+1))
    smeshgui.SetName(idsmZ, "SubMeshEdgeZ_"+str(i+1))

    smeshgui.SetHypothesis(idsmY, idsegY)
    smeshgui.SetHypothesis(idsmZ, idsegZ)

sg.updateObjBrowser(1)

print "-------------------------- compute the mesh of the boxe"
ret=gen.Compute(mesh,idbox)
print ret
if ret != 0:
    log=mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
else:
    print "problem when Computing the mesh"

sg.updateObjBrowser(1)


