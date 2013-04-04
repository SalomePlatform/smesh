# Sew Conform Free Borders


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook


# create two faces of the box
box1 = geompy.MakeBox(0., 0., -10., 20., 20., 15.)
facesList1 = geompy.SubShapeAll(box1, geompy.ShapeType["FACE"])
face1 = facesList1[2]

box2 = geompy.MakeBox(0., 5., 0., 20., 20., 15.)
facesList2 = geompy.SubShapeAll(box2, geompy.ShapeType["FACE"])
face2 = facesList2[1]

edgesList = geompy.SubShapeAll(face2, geompy.ShapeType["EDGE"])
edge1 = edgesList[2]

aComp = geompy.MakeCompound([face1, face2])
geompy.addToStudy(aComp, "Two faces")

# create a mesh on two faces
mesh = smesh.Mesh(aComp, "Two faces : quadrangle mesh")

algo1D = mesh.Segment()
algo1D.NumberOfSegments(9)
algo2D = mesh.Quadrangle()

algo_local = mesh.Segment(edge1)
algo_local.Arithmetic1D(1, 4)
algo_local.Propagation()

mesh.Compute()

# sew conform free borders
# FirstNodeID1, SecondNodeID1, LastNodeID1, FirstNodeID2, SecondNodeID2
mesh.SewConformFreeBorders(5, 45, 6, 3, 24) 
