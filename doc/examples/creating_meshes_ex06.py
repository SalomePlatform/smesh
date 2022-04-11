# Creating a hexahedral mesh on a cylinder.
#
# This example uses Partition to divide the cylinder into blocks, which is
# a general approach. But for the case of cylinder there is a dedicated
# command creating a blocked cylinder: geom_builder.MakeDividedCylinder()

import math

import salome
salome.salome_init_without_session()

import GEOM
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Parameters
# ----------

radius =  50
height = 200

# Build a cylinder
# ----------------

base      = geom_builder.MakeVertex(0, 0, 0)
direction = geom_builder.MakeVectorDXDYDZ(0, 0, 1)

cylinder = geom_builder.MakeCylinder(base, direction, radius, height)

geom_builder.addToStudy(cylinder, "cylinder")

# Build blocks
# ------------

size = radius/2.0

box_rot  = geom_builder.MakeBox(-size, -size, 0,  +size, +size, height)
box_axis = geom_builder.MakeLine(base, direction)
box      = geom_builder.MakeRotation(box_rot, box_axis, math.pi/4)

hole = geom_builder.MakeCut(cylinder, box)

plane_trim = 2000

plane_a = geom_builder.MakePlane(base, geom_builder.MakeVectorDXDYDZ(1, 0, 0), plane_trim)
plane_b = geom_builder.MakePlane(base, geom_builder.MakeVectorDXDYDZ(0, 1, 0), plane_trim)

blocks_part = geom_builder.MakePartition([hole], [plane_a, plane_b], [], [], geom_builder.ShapeType["SOLID"])
blocks_list = [box] + geom_builder.SubShapeAll(blocks_part, geom_builder.ShapeType["SOLID"])
blocks_all  = geom_builder.MakeCompound(blocks_list)
blocks      = geom_builder.MakeGlueFaces(blocks_all, 0.0001)

geom_builder.addToStudy(blocks, "cylinder:blocks")

# Build geometric groups
# ----------------------

group_a = geom_builder.CreateGroup(blocks, geom_builder.ShapeType["FACE"])
geom_builder.addToStudyInFather(blocks, group_a, "baseA")
items = geom_builder.GetShapesOnPlaneWithLocationIDs(blocks, geom_builder.ShapeType["FACE"], direction, base, GEOM.ST_ON)
geom_builder.UnionIDs(group_a, items)

base_b = geom_builder.MakeVertex(0, 0, height)
group_b = geom_builder.CreateGroup(blocks, geom_builder.ShapeType["FACE"])
geom_builder.addToStudyInFather(blocks, group_b, "baseB")
items = geom_builder.GetShapesOnPlaneWithLocationIDs(blocks, geom_builder.ShapeType["FACE"], direction, base_b, GEOM.ST_ON)
geom_builder.UnionIDs(group_b, items)

group_1 = geom_builder.CreateGroup(blocks, geom_builder.ShapeType["SOLID"])
geom_builder.addToStudyInFather(blocks, group_1, "limit")
group_1_all = geom_builder.SubShapeAllIDs(blocks, geom_builder.ShapeType["SOLID"])
geom_builder.UnionIDs(group_1, group_1_all)
group_1_box = geom_builder.GetBlockNearPoint(blocks, base)
geom_builder.DifferenceList(group_1, [group_1_box])

# Mesh the blocks with hexahedral
# -------------------------------

smesh_builder.UpdateStudy()

hexa = smesh_builder.Mesh(blocks)

hexa_1d = hexa.Segment()
hexa_1d.NumberOfSegments(1)

vertex = geom_builder.MakeVertex(+radius, +radius, 0)
edge = geom_builder.GetEdgeNearPoint(blocks, vertex)
algo = hexa.Segment(edge)
algo.NumberOfSegments(5)
algo.Propagation()

vertex = geom_builder.MakeVertex(-radius, +radius, 0)
edge = geom_builder.GetEdgeNearPoint(blocks, vertex)
algo = hexa.Segment(edge)
algo.NumberOfSegments(8)
algo.Propagation()

vertex = geom_builder.MakeVertex((radius+size)/2, 0, 0)
edge = geom_builder.GetEdgeNearPoint(blocks, vertex)
algo = hexa.Segment(edge)
algo.NumberOfSegments(10)
algo.Propagation()

vertex = geom_builder.MakeVertex(+radius, 0, height/2)
edge = geom_builder.GetEdgeNearPoint(blocks, vertex)
algo = hexa.Segment(edge)
algo.NumberOfSegments(20)
algo.Propagation()

hexa.Quadrangle()
hexa.Hexahedron()

hexa.Compute()

hexa.Group(group_a)
hexa.Group(group_b)
hexa.Group(group_1)
