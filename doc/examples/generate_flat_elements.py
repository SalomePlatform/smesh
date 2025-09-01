# Double nodes on groups boundaries

# This example represents an iron cable (a thin cylinder) in a concrete block (a big cylinder).
# The big cylinder is defined by two geometric volumes.

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import GEOM
from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# geometry 

O = geom_builder.MakeVertex(0, 0, 0)
OX = geom_builder.MakeVectorDXDYDZ(1, 0, 0)
OY = geom_builder.MakeVectorDXDYDZ(0, 1, 0)
OZ = geom_builder.MakeVectorDXDYDZ(0, 0, 1)
Vertex_1 = geom_builder.MakeVertex(50, 0, 0)
Cylinder_1 = geom_builder.MakeCylinder(O, OX, 10, 500)
Cylinder_2 = geom_builder.MakeCylinder(Vertex_1, OX, 100, 400)
Vertex_2 = geom_builder.MakeVertex(-200, -200, -200)
Vertex_3 = geom_builder.MakeVertex(250, 200, 200)
Box_1 = geom_builder.MakeBoxTwoPnt(Vertex_2, Vertex_3)
Fuse_1 = geom_builder.MakeFuse(Cylinder_1, Cylinder_2)
Partition_1 = geom_builder.MakePartition([Fuse_1], [Cylinder_1, Box_1], [], [], geom_builder.ShapeType["SOLID"], 0, [], 0)
[Solid_1,Solid_2] = geom_builder.GetShapesOnShape(Cylinder_1, Partition_1, geom_builder.ShapeType["SOLID"], GEOM.ST_IN)
[Solid_3,Solid_4] = geom_builder.GetShapesOnShape(Cylinder_2, Partition_1, geom_builder.ShapeType["SOLID"], GEOM.ST_IN)
Vertex_4 = geom_builder.MakeVertex(450, 0, 0)
Vertex_5 = geom_builder.MakeVertex(500, 0, 0)
Vertex_6 = geom_builder.MakeVertex(550, 0, 0)
vec1 = geom_builder.MakeVector(Vertex_4, Vertex_5)
vec2 = geom_builder.MakeVector(Vertex_5, Vertex_6)
[Face_1] = geom_builder.GetShapesOnPlane(Partition_1, geom_builder.ShapeType["FACE"], vec1, GEOM.ST_ON)
[Face_2] = geom_builder.GetShapesOnPlane(Partition_1, geom_builder.ShapeType["FACE"], vec2, GEOM.ST_ON)

# meshing (we have linear tetrahedrons here, but other elements are OK)

Mesh_1 = smesh_builder.Mesh(Partition_1)
Mesh_1.Segment().NumberOfSegments(15)
Mesh_1.Triangle().LengthFromEdges()
Mesh_1.Tetrahedron()
if not Mesh_1.Compute(): raise Exception("Error when computing Mesh")

# relevant groups of volumes and faces

Solid_1_1 = Mesh_1.GroupOnGeom(Solid_1,'Solid_1',SMESH.VOLUME)
Solid_2_1 = Mesh_1.GroupOnGeom(Solid_2,'Solid_2',SMESH.VOLUME)
Solid_3_1 = Mesh_1.GroupOnGeom(Solid_3,'Solid_3',SMESH.VOLUME)
Solid_4_1 = Mesh_1.GroupOnGeom(Solid_4,'Solid_4',SMESH.VOLUME)
Face_1_1 = Mesh_1.GroupOnGeom(Face_1,'Face_1',SMESH.FACE)
Face_2_1 = Mesh_1.GroupOnGeom(Face_2,'Face_2',SMESH.FACE)

# Building of flat elements

Mesh_1.DoubleNodesOnGroupBoundaries([Solid_1_1, Solid_2_1, Solid_3_1, Solid_4_1], 1)

Mesh_1.CreateFlatElementsOnFacesGroups([Face_1_1, Face_2_1])
