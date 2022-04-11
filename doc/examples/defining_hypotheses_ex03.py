# Start and End Length

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBoxDXDYDZ(10., 10., 10.)
geom_builder.addToStudy(box, "Box")

# get one edge of the box to put local hypothesis on
p5 = geom_builder.MakeVertex(5., 0., 0.)
EdgeX = geom_builder.GetEdgeNearPoint(box, p5)
geom_builder.addToStudyInFather(box, EdgeX, "Edge [0,0,0 - 10,0,0]")

# create a hexahedral mesh on the box
hexa = smesh_builder.Mesh(box, "Box : hexahedrical mesh")

# set algorithms
algo1D = hexa.Segment()
hexa.Quadrangle()
hexa.Hexahedron()

# define "NumberOfSegments" hypothesis to cut an edge in a fixed number of segments
algo1D.NumberOfSegments(4)

# create a local hypothesis
algo_local = hexa.Segment(EdgeX)

# define "StartEndLength" hypothesis to cut an edge in several segments with increasing geometric length
algo_local.StartEndLength(1, 6)

# define "Propagation" hypothesis that propagates all other hypothesis
# on all edges on the opposite side in case of quadrangular faces
algo_local.Propagation()

# compute the mesh
hexa.Compute()
