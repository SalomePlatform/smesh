# Construction of a Submesh

from geompy import *
import smesh

# create a box
box = MakeBoxDXDYDZ(10., 10., 10.)
addToStudy(box, "Box")

# select one edge of the box for definition of a local hypothesis
p5 = MakeVertex(5., 0., 0.)
EdgeX = GetEdgeNearPoint(box, p5)
addToStudyInFather(box, EdgeX, "Edge [0,0,0 - 10,0,0]")

# create a hexahedral mesh on the box
quadra = smesh.Mesh(box, "Box : quadrangle 2D mesh")

# create a regular 1D algorithm for the faces
algo1D = quadra.Segment()

# define "NumberOfSegments" hypothesis to cut
# all the edges in a fixed number of segments
algo1D.NumberOfSegments(4)

# create a quadrangle 2D algorithm for the faces
quadra.Quadrangle()

# construct a submesh on the edge with a local hypothesis
algo_local = quadra.Segment(EdgeX)

# define "Arithmetic1D" hypothesis to cut the edge in several segments with increasing arithmetic length
algo_local.Arithmetic1D(1, 4)

# define "Propagation" hypothesis that propagates all other hypotheses
# on all edges of the opposite side in case of quadrangular faces
algo_local.Propagation()

# compute the mesh
quadra.Compute()
