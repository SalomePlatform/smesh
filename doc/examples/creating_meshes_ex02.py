# Construction of a Sub-mesh

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBoxDXDYDZ(10., 10., 10.)
geom_builder.addToStudy(box, "Box")

# select one edge of the box for definition of a local hypothesis
p5 = geom_builder.MakeVertex(5., 0., 0.)
EdgeX = geom_builder.GetEdgeNearPoint(box, p5)
geom_builder.addToStudyInFather(box, EdgeX, "Edge [0,0,0 - 10,0,0]")

# create a hexahedral mesh on the box
mesh = smesh_builder.Mesh(box, "Box : hexahedral 3D mesh")

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
surfaces = geom_builder.SubShapeAll(box, geom_builder.ShapeType["FACE"])

# method 1: no sub-mesh is created
if not mesh.Compute( surfaces[0] ): raise Exception("Error when computing Mesh")

# method 2: a sub-mesh is created
submesh = mesh.GetSubMesh( surfaces[2], "submesh 2" )
if not submesh.Compute(): raise Exception("Error when computing Mesh")

# compute the whole mesh
if not mesh.Compute(): raise Exception("Error when computing Mesh")
