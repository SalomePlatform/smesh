# Propagation

from geompy import *
import smesh

# create a box
box = MakeBoxDXDYDZ(10., 10., 10.)
addToStudy(box, "Box")

# get one edge of the box to put local hypothesis on
p5 = MakeVertex(5., 0., 0.)
EdgeX = GetEdgeNearPoint(box, p5)
addToStudyInFather(box, EdgeX, "Edge [0,0,0 - 10,0,0]")

# create a hexahedral mesh on the box
hexa = smesh.Mesh(box, "Box : hexahedrical mesh")

# set global algorithms and hypotheses
algo1D = hexa.Segment()
hexa.Quadrangle()
hexa.Hexahedron()
algo1D.NumberOfSegments(4)

# create a sub-mesh with local 1D hypothesis and propagation
algo_local = hexa.Segment(EdgeX)

# define "Arithmetic1D" hypothesis to cut an edge in several segments with increasing length
algo_local.Arithmetic1D(1, 4)

# define "Propagation" hypothesis that propagates all other 1D hypotheses
# from all edges on the opposite side of a face in case of quadrangular faces
algo_local.Propagation()

# compute the mesh
hexa.Compute()
