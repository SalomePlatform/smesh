# Defining Meshing Algorithms

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a box
box = geompy.MakeBoxDXDYDZ(10., 10., 10.)
geompy.addToStudy(box, "Box")

# Create a hexahedral mesh on the box
hexa = smesh.Mesh(box, "Box : hexahedrical mesh")

# create a Regular 1D algorithm for edges
algo1D = hexa.Segment()

# create a quadrangle 2D algorithm for faces
algo2D = hexa.Quadrangle()

# create a hexahedron 3D algorithm for solids
algo3D = hexa.Hexahedron()

# define hypotheses
algo1D.Arithmetic1D(1, 4)

# compute the mesh
hexa.Compute()

# 2. Create a tetrahedral mesh on the box
tetra = smesh.Mesh(box, "Box : tetrahedrical mesh")

# create a Regular 1D algorithm for edges
algo1D = tetra.Segment()

# create a Mefisto 2D algorithm for faces
algo2D = tetra.Triangle()

# create a 3D algorithm for solids
algo3D = tetra.Tetrahedron()

# define hypotheses
algo1D.Arithmetic1D(1, 4)
algo2D.LengthFromEdges()

# compute the mesh
tetra.Compute()
