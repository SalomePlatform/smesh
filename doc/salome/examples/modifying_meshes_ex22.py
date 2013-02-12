# Extrusion

import salome
import geompy

import SMESH_mechanic

smesh = SMESH_mechanic.smesh
mesh = SMESH_mechanic.mesh 

# select the top face
faces = geompy.SubShapeAllSorted(SMESH_mechanic.shape_mesh, geompy.ShapeType["FACE"])
face = faces[7]
geompy.addToStudyInFather(SMESH_mechanic.shape_mesh, face, "face circular top")

# create a vector for extrusion
point = smesh.PointStruct(0., 0., 5.)
vector = smesh.DirStruct(point)

# create a group to be extruded
GroupTri = mesh.GroupOnGeom(face, "Group of faces (extrusion)", smesh.FACE)

# perform extrusion of the group
mesh.ExtrusionSweepObject(GroupTri, vector, 5)

salome.sg.updateObjBrowser(1)
