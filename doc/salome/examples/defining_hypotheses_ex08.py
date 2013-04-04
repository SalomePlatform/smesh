# Propagation

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
