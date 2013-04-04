# Start and End Length

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

# get one edge of the box to put local hypothesis on
p5 = geompy.MakeVertex(5., 0., 0.)
EdgeX = geompy.GetEdgeNearPoint(box, p5)
geompy.addToStudyInFather(box, EdgeX, "Edge [0,0,0 - 10,0,0]")

# create a hexahedral mesh on the box
hexa = smesh.Mesh(box, "Box : hexahedrical mesh")

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
