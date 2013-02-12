# Arithmetic 1D

import geompy
import smesh

# create a box
box = geompy.MakeBoxDXDYDZ(10., 10., 10.)
geompy.addToStudy(box, "Box")

# create a hexahedral mesh on the box
hexa = smesh.Mesh(box, "Box : hexahedrical mesh")

# create a Regular 1D algorithm for edges
algo1D = hexa.Segment()

# optionally reverse node distribution on certain edges
allEdges = geompy.SubShapeAllSortedIDs( box, geompy.ShapeType["EDGE"])
reversedEdges = [ allEdges[0], allEdges[4] ]

# define "Arithmetic1D" hypothesis to cut all edges in several segments with increasing arithmetic length 
algo1D.Arithmetic1D(1, 4, reversedEdges)

# create a quadrangle 2D algorithm for faces
hexa.Quadrangle()

# create a hexahedron 3D algorithm for solids
hexa.Hexahedron()

# compute the mesh
hexa.Compute()
