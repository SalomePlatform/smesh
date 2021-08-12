# Creating a hexahedral mesh on a cylinder.
#
# This example uses Partition to divide the cylinder into blocks, which is
# a general approach. But for the case of cylinder there is a dedicated
# command creating a blocked cylinder: geompy.MakeDividedCylinder()

import salome
salome.salome_init_without_session()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

import math

# Parameters
# ----------

radius =  50
height = 200

# Build a cylinder
# ----------------

base      = geompy.MakeVertex(0, 0, 0)
direction = geompy.MakeVectorDXDYDZ(0, 0, 1)

cylinder = geompy.MakeCylinder(base, direction, radius, height)

geompy.addToStudy(cylinder, "cylinder")

# Build blocks
# ------------

size = radius/2.0

box_rot  = geompy.MakeBox(-size, -size, 0,  +size, +size, height)
box_axis = geompy.MakeLine(base, direction)
box      = geompy.MakeRotation(box_rot, box_axis, math.pi/4)

hole = geompy.MakeCut(cylinder, box)

plane_trim = 2000

plane_a = geompy.MakePlane(base, geompy.MakeVectorDXDYDZ(1, 0, 0), plane_trim)
plane_b = geompy.MakePlane(base, geompy.MakeVectorDXDYDZ(0, 1, 0), plane_trim)

blocks_part = geompy.MakePartition([hole], [plane_a, plane_b], [], [], geompy.ShapeType["SOLID"])
blocks_list = [box] + geompy.SubShapeAll(blocks_part, geompy.ShapeType["SOLID"])
blocks_all  = geompy.MakeCompound(blocks_list)
blocks      = geompy.MakeGlueFaces(blocks_all, 0.0001)

geompy.addToStudy(blocks, "cylinder:blocks")

# Build geometric groups
# ----------------------

group_a = geompy.CreateGroup(blocks, geompy.ShapeType["FACE"])
geompy.addToStudyInFather(blocks, group_a, "baseA")
items = geompy.GetShapesOnPlaneWithLocationIDs(blocks, geompy.ShapeType["FACE"], direction, base, GEOM.ST_ON)
geompy.UnionIDs(group_a, items)

base_b = geompy.MakeVertex(0, 0, height)
group_b = geompy.CreateGroup(blocks, geompy.ShapeType["FACE"])
geompy.addToStudyInFather(blocks, group_b, "baseB")
items = geompy.GetShapesOnPlaneWithLocationIDs(blocks, geompy.ShapeType["FACE"], direction, base_b, GEOM.ST_ON)
geompy.UnionIDs(group_b, items)

group_1 = geompy.CreateGroup(blocks, geompy.ShapeType["SOLID"])
geompy.addToStudyInFather(blocks, group_1, "limit")
group_1_all = geompy.SubShapeAllIDs(blocks, geompy.ShapeType["SOLID"])
geompy.UnionIDs(group_1, group_1_all)
group_1_box = geompy.GetBlockNearPoint(blocks, base)
geompy.DifferenceList(group_1, [group_1_box])

# Mesh the blocks with hexahedral
# -------------------------------

smesh.UpdateStudy()

hexa = smesh.Mesh(blocks)

hexa_1d = hexa.Segment()
hexa_1d.NumberOfSegments(1)

vertex = geompy.MakeVertex(+radius, +radius, 0)
edge = geompy.GetEdgeNearPoint(blocks, vertex)
algo = hexa.Segment(edge)
algo.NumberOfSegments(5)
algo.Propagation()

vertex = geompy.MakeVertex(-radius, +radius, 0)
edge = geompy.GetEdgeNearPoint(blocks, vertex)
algo = hexa.Segment(edge)
algo.NumberOfSegments(8)
algo.Propagation()

vertex = geompy.MakeVertex((radius+size)/2, 0, 0)
edge = geompy.GetEdgeNearPoint(blocks, vertex)
algo = hexa.Segment(edge)
algo.NumberOfSegments(10)
algo.Propagation()

vertex = geompy.MakeVertex(+radius, 0, height/2)
edge = geompy.GetEdgeNearPoint(blocks, vertex)
algo = hexa.Segment(edge)
algo.NumberOfSegments(20)
algo.Propagation()

hexa.Quadrangle()
hexa.Hexahedron()

hexa.Compute()

hexa.Group(group_a)
hexa.Group(group_b)
hexa.Group(group_1)
