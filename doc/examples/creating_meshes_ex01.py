# Construction of a Mesh

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geom_builder.addToStudy(box, "box")

# create a mesh
tetra = smesh_builder.Mesh(box, "MeshBox")

algo1D = tetra.Segment()
algo1D.NumberOfSegments(7)

algo2D = tetra.Triangle()
algo2D.MaxElementArea(800.)

algo3D = tetra.Tetrahedron()
algo3D.MaxElementVolume(900.)

# compute the mesh
ret = tetra.Compute()
if not ret:
    raise Exception("problem when computing the mesh")

print("mesh computed")
