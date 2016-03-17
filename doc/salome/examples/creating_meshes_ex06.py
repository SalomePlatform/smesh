# Creating a hexahedral mesh on a cylinder.
#
# This example uses Partition to divide the cylinder into blocks, which is
# a general approach. But for the case of cylinder there is a dedicated
# command creating a blocked cylinder: geompy.MakeDividedCylinder()

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

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

def group(name, shape, type, base=None, direction=None):
    t = geompy.ShapeType[type]
    g = geompy.CreateGroup(shape, t)

    geompy.addToStudyInFather(shape, g, name)

    if base!=None:
        l = geompy.GetShapesOnPlaneWithLocationIDs(shape, t, direction, base, GEOM.ST_ON)
        geompy.UnionIDs(g, l)

    return g

group_a = group("baseA", blocks, "FACE", base, direction)

base_b  = geompy.MakeVertex(0, 0, height)
group_b = group("baseB", blocks, "FACE", base_b, direction)

group_1     = group("limit", blocks, "SOLID")
group_1_all = geompy.SubShapeAllIDs(blocks, geompy.ShapeType["SOLID"])
geompy.UnionIDs(group_1, group_1_all)
group_1_box = geompy.GetBlockNearPoint(blocks, base)
geompy.DifferenceList(group_1, [group_1_box])

# Mesh the blocks with hexahedral
# -------------------------------

smesh.SetCurrentStudy(salome.myStudy)

def discretize(x, y, z,  nbSeg, shape=blocks):
    vert = geompy.MakeVertex( x, y, z )
    edge = geompy.GetEdgeNearPoint( shape, vert )
    algo = hexa.Segment( edge )
    algo.NumberOfSegments( nbSeg )
    algo.Propagation()

hexa = smesh.Mesh(blocks)

hexa_1d = hexa.Segment()
hexa_1d.NumberOfSegments(1)

discretize(+radius        , +radius,        0,   5)
discretize(-radius        , +radius,        0,   8)
discretize((radius+size)/2,       0,        0,  10)
discretize(        +radius,       0, height/2,  20)

hexa.Quadrangle()
hexa.Hexahedron()

hexa.Compute()

hexa.Group(group_a)
hexa.Group(group_b)
hexa.Group(group_1)
