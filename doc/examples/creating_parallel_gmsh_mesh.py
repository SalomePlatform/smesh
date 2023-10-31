# contains function to compute a mesh in parallel
import salome

salome.salome_init()
import salome_notebook
notebook = salome_notebook.NoteBook()

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder
import SALOMEDS

import numpy as np

geompy = geomBuilder.New()


nbox = 2
boxsize = 100
offset = 0
# Create 3D faces
boxes = []
# First creating all the boxes
for i in range(nbox):
    for j in range(nbox):
        for k in range(nbox):

            x_orig = i*(boxsize+offset)
            y_orig = j*(boxsize+offset)
            z_orig = k*(boxsize+offset)

            tmp_box = geompy.MakeBoxDXDYDZ(boxsize, boxsize, boxsize)

            if not i == j == k == 0:
                box = geompy.MakeTranslation(tmp_box, x_orig,
                                             y_orig, z_orig)
            else:
                box = tmp_box

            geompy.addToStudy(box, 'box_{}:{}:{}'.format(i, j, k))

            boxes.append(box)

# Create fuse of all boxes
all_boxes = geompy.MakeCompound(boxes)
geompy.addToStudy(all_boxes, 'Compound_1')

# Removing duplicates faces and edges
all_boxes = geompy.MakeGlueFaces(all_boxes, 1e-07)
geompy.addToStudy(all_boxes, 'Glued_Faces_1')

rubik_cube = geompy.MakeGlueEdges(all_boxes, 1e-07)
geompy.addToStudy(all_boxes, 'rubik_cube')

smesh = smeshBuilder.New()
print("Creating Parallel Mesh")
par_mesh = smesh.ParallelMesh(rubik_cube, name="par_mesh", mesher3D="GMSH")

print("Creating hypoehtesis for netgen")
NETGEN_3D_Parameters_1 = smesh.CreateHypothesisByAverageLength( 'NETGEN_Parameters',
                                         'NETGENEngine', 34.641, 0 )
print("Adding hypothesis")
par_mesh.AddGlobalHypothesis(NETGEN_3D_Parameters_1, mesher="GMSH" )

#Set here particular mesh parameters in 3D
for algo3d in par_mesh._algo3d:
    param3d = algo3d.Parameters()
    param3d.Set2DAlgo(0)
    param3d.Set3DAlgo(0)
    param3d.SetSmouthSteps(2)
    param3d.SetSizeFactor(1.1)

print("Setting parallelism method")
par_mesh.SetParallelismMethod(smeshBuilder.MULTITHREAD)

print("Setting parallelism options")
param = par_mesh.GetParallelismSettings()
param.SetNbThreads(6)

print("Starting parallel compute")
is_done = par_mesh.Compute()
if not is_done:
    raise Exception("Error when computing Mesh")

print("  Tetrahedron: ",  par_mesh.NbTetras())
print("  Triangle: ", par_mesh.NbTriangles())
print("  edge: ", par_mesh.NbEdges())

assert  par_mesh.NbTetras() > 0
assert  par_mesh.NbTriangles() > 0
assert  par_mesh.NbEdges() > 0
