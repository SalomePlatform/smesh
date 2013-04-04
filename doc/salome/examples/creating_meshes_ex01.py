# Construction of a Mesh

import geompy
import smesh

# create a box
box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

# create a mesh
tetra = smesh.Mesh(box, "MeshBox")

algo1D = tetra.Segment()
algo1D.NumberOfSegments(7)

algo2D = tetra.Triangle()
algo2D.MaxElementArea(800.)

algo3D = tetra.Tetrahedron()
algo3D.MaxElementVolume(900.)

# compute the mesh
ret = tetra.Compute()
if ret == 0:
    print "problem when computing the mesh"
else:
    print "mesh computed"
    pass