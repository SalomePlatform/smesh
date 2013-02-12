# Smoothing

import salome
import geompy

import SMESH_mechanic

smesh = SMESH_mechanic.smesh
mesh = SMESH_mechanic.mesh

# select the top face
faces = geompy.SubShapeAllSorted(SMESH_mechanic.shape_mesh, geompy.ShapeType["FACE"])
face = faces[3]
geompy.addToStudyInFather(SMESH_mechanic.shape_mesh, face, "face planar with hole")

# create a group of faces to be smoothed
GroupSmooth = mesh.GroupOnGeom(face, "Group of faces (smooth)", smesh.FACE)

# perform smoothing

# boolean SmoothObject(Object, IDsOfFixedNodes, MaxNbOfIterations, MaxAspectRatio, Method)
res = mesh.SmoothObject(GroupSmooth, [], 20, 2., smesh.CENTROIDAL_SMOOTH)
print "\nSmoothing ... ",
if not res: print "failed!"
else:       print "done."

salome.sg.updateObjBrowser(1) 
