# Create a Group on Geometry


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a box
box = geompy.MakeBox(0., 0., 0., 100., 100., 100.)
geompy.addToStudy(box, "box")

# add the first face of the box to the study
subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
face = subShapeList[0]
geompy.addToStudyInFather(box, face, "face 1") 

# create group of edges on the face
aGeomGroupE = geompy.CreateGroup(face, geompy.ShapeType["EDGE"])
geompy.AddObject(aGeomGroupE, 3)
geompy.AddObject(aGeomGroupE, 6)
geompy.AddObject(aGeomGroupE, 8)
geompy.AddObject(aGeomGroupE, 10)
geompy.addToStudyInFather(face, aGeomGroupE, "Group of Edges")

# create quadrangle 2D mesh on the box
quadra = smesh.Mesh(box, "Box : quadrangle 2D mesh")
algo1D = quadra.Segment()
quadra.Quadrangle()
algo1D.NumberOfSegments(7) 

# compute the mesh
quadra.Compute()

# create SMESH group on the face with name "SMESHGroup1"
aSmeshGroup1 = quadra.GroupOnGeom(face, "SMESHGroup1")

# create SMESH group on <aGeomGroupE> with default name
aSmeshGroup2 = quadra.GroupOnGeom(aGeomGroupE) 

salome.sg.updateObjBrowser(True)
