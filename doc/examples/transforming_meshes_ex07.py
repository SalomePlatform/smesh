# Sew Meshes Border to Side

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create two faces of a box
box1 = geom_builder.MakeBox(0., 0., -10., 30., 20., 25.)
facesList1 = geom_builder.SubShapeAll(box1, geom_builder.ShapeType["FACE"])
face1 = facesList1[2]

box2 = geom_builder.MakeBox(0., 5., 0., 20., 20., 15.)
facesList2 = geom_builder.SubShapeAll(box2, geom_builder.ShapeType["FACE"])
face2 = facesList2[1]

edgesList = geom_builder.SubShapeAll(face2, geom_builder.ShapeType["EDGE"])
edge1 = edgesList[2]

aComp = geom_builder.MakeCompound([face1, face2])
geom_builder.addToStudy(aComp, "Two faces")

# create a mesh on two faces
mesh = smesh_builder.Mesh(aComp, "Two faces : quadrangle mesh")

algo1D = mesh.Segment()
algo1D.NumberOfSegments(9)
algo2D = mesh.Quadrangle()

algo_local = mesh.Segment(edge1)
algo_local.Arithmetic1D(1, 4)
algo_local.Propagation()

mesh.Compute()

# sew border to side
# FirstNodeIDOnFreeBorder, SecondNodeIDOnFreeBorder, LastNodeIDOnFreeBorder,
# FirstNodeIDOnSide, LastNodeIDOnSide,
# CreatePolygons, CreatePolyedrs
mesh.SewBorderToSide(5, 45, 6, 113, 109, 0, 0)
