# Defining Meshing Algorithms

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBoxDXDYDZ(10., 10., 10.)
geom_builder.addToStudy(box, "Box")

# Create a hexahedral mesh on the box
hexa = smesh_builder.Mesh(box, "Box : hexahedrical mesh")

# create a Regular 1D algorithm for edges
algo1D = hexa.Segment()

# create a quadrangle 2D algorithm for faces
algo2D = hexa.Quadrangle()

# create a hexahedron 3D algorithm for solids
algo3D = hexa.Hexahedron()

# define hypotheses
algo1D.Arithmetic1D(1, 4)

# compute the mesh
if not hexa.Compute(): raise Exception("Error when computing Mesh")

# 2. Create a tetrahedral mesh on the box
tetra = smesh_builder.Mesh(box, "Box : tetrahedrical mesh")

# create a 1D algorithm for edges
algo1D = tetra.Segment()

# create a 2D algorithm for faces
algo2D = tetra.Triangle()

# create a 3D algorithm for solids
algo3D = tetra.Tetrahedron()

# define hypotheses
algo1D.Arithmetic1D(1, 4)
algo2D.LengthFromEdges()

# compute the mesh
if not tetra.Compute(): raise Exception("Error when computing Mesh")
