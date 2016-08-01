# Construction of a Sub-mesh

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

# select one edge of the box for definition of a local hypothesis
p5 = geompy.MakeVertex(5., 0., 0.)
EdgeX = geompy.GetEdgeNearPoint(box, p5)
geompy.addToStudyInFather(box, EdgeX, "Edge [0,0,0 - 10,0,0]")

# create a hexahedral mesh on the box
mesh = smesh.Mesh(box, "Box : hexahedral 3D mesh")

# create a Regular_1D algorithm for discretization of edges
algo1D = mesh.Segment()

# define "NumberOfSegments" hypothesis to cut
# all the edges in a fixed number of segments
algo1D.NumberOfSegments(4)

# create a quadrangle 2D algorithm for the faces
mesh.Quadrangle()

# construct a sub-mesh on the edge with a local Regular_1D algorithm
algo_local = mesh.Segment(EdgeX)

# define "Arithmetic1D" hypothesis to cut EdgeX in several segments with length arithmetically
# increasing from 1.0 to 4.0
algo_local.Arithmetic1D(1, 4)

# define "Propagation" hypothesis that propagates algo_local and "Arithmetic1D" hypothesis
# from EdgeX to all parallel edges
algo_local.Propagation()

# assign a hexahedral algorithm
mesh.Hexahedron()


# any sub-shape can be meshed individually --
# compute mesh on two surfaces using different methods

# get surfaces
surfaces = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])

# method 1: no sub-mesh is created
mesh.Compute( surfaces[0] )

# method 2: a sub-mesh is created
submesh = mesh.GetSubMesh( surfaces[2], "submesh 2" )
submesh.Compute()



# compute the whole mesh
mesh.Compute()
