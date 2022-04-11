# Usage of Segments around Vertex algorithm

# for meshing a box with quadrangles with refinement near vertices

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBoxDXDYDZ( 10, 10, 10 )

# make a mesh
mesh = smesh_builder.Mesh( box )

# define quadrangle meshing
algo1d = mesh.Segment()
algo1d.LocalLength( 1. )
mesh.Quadrangle()

# add Hexahedron algo to assure that there are no triangles
mesh.Hexahedron()

# define refinement near vertices
algo1d.LengthNearVertex( 0.2 )

mesh.Compute()
