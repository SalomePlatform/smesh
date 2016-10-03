# Arithmetic Progression and Geometric Progression

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a box
box = geompy.MakeBoxDXDYDZ(10., 10., 10.)
geompy.addToStudy(box, "Box")

# create a hexahedral mesh on the box
hexa = smesh.Mesh(box, "Box : hexahedrical mesh")

# create a Regular 1D algorithm for edges
algo1D = hexa.Segment()

# optionally reverse node distribution on certain edges
allEdges = geompy.SubShapeAllSorted( box, geompy.ShapeType["EDGE"])
reversedEdges = [ allEdges[0], allEdges[4] ]

# define "Arithmetic1D" hypothesis to cut all edges in several segments with increasing arithmetic length 
algo1D.Arithmetic1D(1, 4, reversedEdges)

# define "Geometric Progression" hypothesis on one edge to cut this edge in segments with length increasing by 20% starting from 1
gpAlgo = hexa.Segment( allEdges[1] )
gpAlgo.GeometricProgression( 1, 1.2 )

# propagate distribution of nodes computed using "Geometric Progression" to parallel edges
gpAlgo.PropagationOfDistribution() 


# create a quadrangle 2D algorithm for faces
hexa.Quadrangle()

# create a hexahedron 3D algorithm for solids
hexa.Hexahedron()

# compute the mesh
hexa.Compute()
