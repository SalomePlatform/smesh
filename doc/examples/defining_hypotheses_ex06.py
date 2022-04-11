# Maximum Element Volume

import salome
salome.salome_init_without_session()

from salome.smesh import smeshBuilder
from salome.geom import geomBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a cylinder
cyl = geom_builder.MakeCylinderRH(30., 50.)
geom_builder.addToStudy(cyl, "cyl")

# create a mesh on the cylinder
tetra = smesh_builder.Mesh(cyl, "Cylinder : tetrahedrical mesh")

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
    print("problem when computing the mesh")
else:
    print("Computation succeeded")
