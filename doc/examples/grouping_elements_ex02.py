# Create a Group on Geometry

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBox(0., 0., 0., 100., 100., 100.)
geom_builder.addToStudy(box, "box")

# add the first face of the box to the study
subShapeList = geom_builder.SubShapeAll(box, geom_builder.ShapeType["FACE"])
face = subShapeList[0]
geom_builder.addToStudyInFather(box, face, "face 1") 

# create group of edges on the face
aGeomGroupE = geom_builder.CreateGroup(face, geom_builder.ShapeType["EDGE"])
geom_builder.AddObject(aGeomGroupE, 3)
geom_builder.AddObject(aGeomGroupE, 6)
geom_builder.AddObject(aGeomGroupE, 8)
geom_builder.AddObject(aGeomGroupE, 10)
geom_builder.addToStudyInFather(face, aGeomGroupE, "Group of Edges")

# create quadrangle 2D mesh on the box
quadra = smesh_builder.Mesh(box, "Box : quadrangle 2D mesh")
algo1D = quadra.Segment()
quadra.Quadrangle()
algo1D.NumberOfSegments(7) 

# compute the mesh
quadra.Compute()

# create SMESH group on the face with name "SMESHGroup1"
aSmeshGroup1 = quadra.GroupOnGeom(face, "SMESHGroup1")

# create SMESH group on <aGeomGroupE> with default name
aSmeshGroup2 = quadra.GroupOnGeom(aGeomGroupE) 
