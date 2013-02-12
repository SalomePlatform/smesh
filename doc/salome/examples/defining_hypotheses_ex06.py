# Maximum Element Volume

import geompy
import smesh

# create a cylinder
cyl = geompy.MakeCylinderRH(30., 50.)
geompy.addToStudy(cyl, "cyl")

# create a mesh on the cylinder
tetra = smesh.Mesh(cyl, "Cylinder : tetrahedrical mesh")

# assign algorithms
algo1D = tetra.Segment()
algo2D = tetra.Triangle()
algo3D = tetra.Tetrahedron()

# assign 1D and 2D hypotheses
algo1D.NumberOfSegments(7)
algo2D.MaxElementArea(150.)

# assign Max Element Volume hypothesis
algo3D.MaxElementVolume(200.)

# compute the mesh
ret = tetra.Compute()
if ret == 0:
    print "probleme when computing the mesh"
else:
    print "Computation succeded"
