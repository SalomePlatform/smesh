# Double nodes on groups boundaries

# This example represents an iron cable (a thin cylinder) in a concrete bloc (a big cylinder).
# The big cylinder is defined by two geometric volumes.


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# geometry 

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Vertex_1 = geompy.MakeVertex(50, 0, 0)
Cylinder_1 = geompy.MakeCylinder(O, OX, 10, 500)
Cylinder_2 = geompy.MakeCylinder(Vertex_1, OX, 100, 400)
Vertex_2 = geompy.MakeVertex(-200, -200, -200)
Vertex_3 = geompy.MakeVertex(250, 200, 200)
Box_1 = geompy.MakeBoxTwoPnt(Vertex_2, Vertex_3)
Fuse_1 = geompy.MakeFuse(Cylinder_1, Cylinder_2)
Partition_1 = geompy.MakePartition([Fuse_1], [Cylinder_1, Box_1], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
[Solid_1,Solid_2] = geompy.GetShapesOnShape(Cylinder_1, Partition_1, geompy.ShapeType["SOLID"], GEOM.ST_IN)
[Solid_3,Solid_4] = geompy.GetShapesOnShape(Cylinder_2, Partition_1, geompy.ShapeType["SOLID"], GEOM.ST_IN)
Vertex_4 = geompy.MakeVertex(450, 0, 0)
Vertex_5 = geompy.MakeVertex(500, 0, 0)
Vertex_6 = geompy.MakeVertex(550, 0, 0)
vec1 = geompy.MakeVector(Vertex_4, Vertex_5)
vec2 = geompy.MakeVector(Vertex_5, Vertex_6)
[Face_1] = geompy.GetShapesOnPlane(Partition_1, geompy.ShapeType["FACE"], vec1, GEOM.ST_ON)
[Face_2] = geompy.GetShapesOnPlane(Partition_1, geompy.ShapeType["FACE"], vec2, GEOM.ST_ON)

# meshing (we have linear tetrahedrons here, but other elements are OK)

Mesh_1 = smesh.Mesh(Partition_1)
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15)
MEFISTO_2D = Mesh_1.Triangle(algo=smeshBuilder.MEFISTO)
Length_From_Edges_2D = MEFISTO_2D.LengthFromEdges()
ALGO3D = Mesh_1.Tetrahedron()
isDone = Mesh_1.Compute()

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
