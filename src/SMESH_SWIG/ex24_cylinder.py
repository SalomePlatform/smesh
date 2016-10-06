#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# ==================================
#
import math

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

geo = geompy

# Parameters
# ----------

radius =  50
height = 200

# Build a cylinder
# ----------------

base = geompy.MakeVertex(0, 0, 0)
direction = geompy.MakeVectorDXDYDZ(0, 0, 1)

cylinder = geompy.MakeCylinder(base, direction, radius, height)

geompy.addToStudy(cylinder, "cylinder")

# Build blocks
# ------------

size = radius/2.0

box_rot = geompy.MakeBox(-size, -size, 0,  +size, +size, height)
box_axis = geompy.MakeLine(base, direction)
box = geompy.MakeRotation(box_rot, box_axis, math.pi/4)

hole = geompy.MakeCut(cylinder, box)

plane_trim = 2000

plane_a = geompy.MakePlane(base, geompy.MakeVectorDXDYDZ(1, 0, 0), plane_trim)
plane_b = geompy.MakePlane(base, geompy.MakeVectorDXDYDZ(0, 1, 0), plane_trim)

blocks_part = geompy.MakePartition([hole], [plane_a, plane_b], [], [], geompy.ShapeType["SOLID"])
blocks_list = [box] + geompy.SubShapeAll(blocks_part, geompy.ShapeType["SOLID"])
blocks_all = geompy.MakeCompound(blocks_list)
blocks = geompy.MakeGlueFaces(blocks_all, 0.0001)

geompy.addToStudy(blocks, "cylinder:blocks")

# Build geometric groups
# ----------------------

def group(name, shape, type, base=None, direction=None):
    t = geompy.ShapeType[type]
    g = geompy.CreateGroup(shape, t)

    geompy.addToStudy(g, name)
    g.SetName(name)

    if base!=None:
        l = geompy.GetShapesOnPlaneWithLocationIDs(shape, t, direction, base, GEOM.ST_ON)
        geompy.UnionIDs(g, l)

    return g

group_a = group("baseA", blocks, "FACE", base, direction)

base_b  = geompy.MakeVertex(0, 0, height)
group_b = group("baseB", blocks, "FACE", base_b, direction)

group_1 = group("limit", blocks, "SOLID")
group_1_all = geompy.SubShapeAllIDs(blocks, geompy.ShapeType["SOLID"])
geompy.UnionIDs(group_1, group_1_all)
group_1_box = geompy.GetBlockNearPoint(blocks, base)
geompy.DifferenceList(group_1, [group_1_box])

# Mesh the blocks with hexahedral
# -------------------------------

smesh.SetCurrentStudy(salome.myStudy)

def discretize(x, y, z,  n, s=blocks):
    p = geompy.MakeVertex(x, y, z)
    e = geompy.GetEdgeNearPoint(s, p)
    a = hexa.Segment(e)
    a.NumberOfSegments(n)
    a.Propagation()

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

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
