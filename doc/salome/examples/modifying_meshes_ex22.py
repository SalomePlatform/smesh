# Extrusion


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

import SMESH_mechanic

#smesh = SMESH_mechanic.smesh
mesh = SMESH_mechanic.mesh 

# select the top face
faces = geompy.SubShapeAllSorted(SMESH_mechanic.shape_mesh, geompy.ShapeType["FACE"])
face = faces[7]
geompy.addToStudyInFather(SMESH_mechanic.shape_mesh, face, "face circular top")

# create a vector for extrusion
point = SMESH.PointStruct(0., 0., 5.)
vector = SMESH.DirStruct(point)

# create a group to be extruded
GroupTri = mesh.GroupOnGeom(face, "Group of faces (extrusion)", SMESH.FACE)

# perform extrusion of the group
mesh.ExtrusionSweepObject(GroupTri, vector, 5)

salome.sg.updateObjBrowser(True)
