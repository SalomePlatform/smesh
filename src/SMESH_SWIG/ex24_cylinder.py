#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

import geompy
import smesh
import salome
geo = geompy

# Parameters
# ----------

radius =  50
height = 200

# Build a cylinder
# ----------------

base = geo.MakeVertex(0, 0, 0)
direction = geo.MakeVectorDXDYDZ(0, 0, 1)

cylinder = geo.MakeCylinder(base, direction, radius, height)

geo.addToStudy(cylinder, "cylinder")

# Build blocks
# ------------

size = radius/2.0

box_rot = geo.MakeBox(-size, -size, 0,  +size, +size, height)
box_axis = geo.MakeLine(base, direction)
box = geo.MakeRotation(box_rot, box_axis, math.pi/4)

hole = geo.MakeCut(cylinder, box)

plane_trim = 2000

plane_a = geo.MakePlane(base, geo.MakeVectorDXDYDZ(1, 0, 0), plane_trim)
plane_b = geo.MakePlane(base, geo.MakeVectorDXDYDZ(0, 1, 0), plane_trim)

blocks_part = geo.MakePartition([hole], [plane_a, plane_b], [], [], geo.ShapeType["SOLID"])
blocks_list = [box] + geo.SubShapeAll(blocks_part, geo.ShapeType["SOLID"])
blocks_all = geo.MakeCompound(blocks_list)
blocks = geo.MakeGlueFaces(blocks_all, 0.0001)

geo.addToStudy(blocks, "cylinder:blocks")

# Build geometric groups
# ----------------------

def group(name, shape, type, base=None, direction=None):
    t = geo.ShapeType[type]
    g = geo.CreateGroup(shape, t)

    geo.addToStudy(g, name)
    g.SetName(name)

    if base!=None:
        l = geo.GetShapesOnPlaneWithLocationIDs(shape, t, direction, base, geo.GEOM.ST_ON)
        geo.UnionIDs(g, l)

    return g

group_a = group("baseA", blocks, "FACE", base, direction)

base_b  = geo.MakeVertex(0, 0, height)
group_b = group("baseB", blocks, "FACE", base_b, direction)

group_1 = group("limit", blocks, "SOLID")
group_1_all = geo.SubShapeAllIDs(blocks, geo.ShapeType["SOLID"])
geo.UnionIDs(group_1, group_1_all)
group_1_box = geo.GetBlockNearPoint(blocks, base)
geo.DifferenceList(group_1, [group_1_box])

# Mesh the blocks with hexahedral
# -------------------------------

smesh.SetCurrentStudy(salome.myStudy)

def discretize(x, y, z,  n, s=blocks):
    p = geo.MakeVertex(x, y, z)
    e = geo.GetEdgeNearPoint(s, p)
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
