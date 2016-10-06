#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Geometry
# ========

# With hexahedral, build a box partially holed by a not centered cylinder with a thickness

# Values
# ------

box_dx = 1000
box_dy =  900
box_dz =  800

cyl_x      = 500
cyl_y      = 300
cyl_dz     = 600
cyl_radius = 150
cyl_thick  =  30

# Triangular face
# ---------------

def triangle(p1, p2, p3):
    l = []
    l.append(geompy.MakeEdge(p1, p2))
    l.append(geompy.MakeEdge(p2, p3))
    l.append(geompy.MakeEdge(p3, p1))
    w = geompy.MakeWire(l)
    return geompy.MakeFace(w, 1)

# The holed part
# ==============

# Vertex of the holed part
# ------------------------

hole_point_11 = geompy.MakeVertex(0     , 0     , 0)
hole_point_21 = geompy.MakeVertex(box_dx, 0     , 0)
hole_point_12 = geompy.MakeVertex(0     , box_dy, 0)
hole_point_22 = geompy.MakeVertex(box_dx, box_dy, 0)

hole_center   = geompy.MakeVertex(cyl_x, cyl_y, 0)

# Faces of the holed part
# -----------------------

hole_face_1 = triangle(hole_point_11, hole_point_21, hole_center)
hole_face_2 = triangle(hole_point_21, hole_point_22, hole_center)
hole_face_3 = triangle(hole_point_12, hole_point_22, hole_center)
hole_face_4 = triangle(hole_point_11, hole_point_12, hole_center)

# Solids of the holed part
# ------------------------

cyl_dir = geompy.MakeVectorDXDYDZ(0, 0, 1)

hole_solid_1 = geompy.MakePrismVecH(hole_face_1, cyl_dir, cyl_dz)
hole_solid_2 = geompy.MakePrismVecH(hole_face_2, cyl_dir, cyl_dz)
hole_solid_3 = geompy.MakePrismVecH(hole_face_3, cyl_dir, cyl_dz)
hole_solid_4 = geompy.MakePrismVecH(hole_face_4, cyl_dir, cyl_dz)

hole_internal = geompy.MakeCylinder(hole_center, cyl_dir, cyl_radius          , cyl_dz)
hole_external = geompy.MakeCylinder(hole_center, cyl_dir, cyl_radius+cyl_thick, cyl_dz)
hole_median   = geompy.MakeCut(hole_external, hole_internal)

# Boolean operations
# ------------------

blocks = []

blocks.append(   geompy.MakeCut(hole_solid_1, hole_external))
blocks.append(geompy.MakeCommon(hole_solid_1, hole_median  ))

blocks.append(   geompy.MakeCut(hole_solid_2, hole_external))
blocks.append(geompy.MakeCommon(hole_solid_2, hole_median  ))

blocks.append(   geompy.MakeCut(hole_solid_3, hole_external))
blocks.append(geompy.MakeCommon(hole_solid_3, hole_median  ))

blocks.append(   geompy.MakeCut(hole_solid_4, hole_external))
blocks.append(geompy.MakeCommon(hole_solid_4, hole_median  ))

# The full part
# =============

# Vertex of the full part
# -----------------------

full_point_11 = geompy.MakeVertex(0     , 0     , cyl_dz)
full_point_21 = geompy.MakeVertex(box_dx, 0     , cyl_dz)
full_point_12 = geompy.MakeVertex(0     , box_dy, cyl_dz)
full_point_22 = geompy.MakeVertex(box_dx, box_dy, cyl_dz)

full_center = geompy.MakeVertex(cyl_x, cyl_y, cyl_dz)

# Faces of the full part
# ----------------------

full_face_1 = triangle(full_point_11, full_point_21, full_center)
full_face_2 = triangle(full_point_21, full_point_22, full_center)
full_face_3 = triangle(full_point_12, full_point_22, full_center)
full_face_4 = triangle(full_point_11, full_point_12, full_center)

# Solids of the full part
# ------------------------

full_dz = box_dz - cyl_dz

full_solid_1 = geompy.MakePrismVecH(full_face_1, cyl_dir, full_dz)
full_solid_2 = geompy.MakePrismVecH(full_face_2, cyl_dir, full_dz)
full_solid_3 = geompy.MakePrismVecH(full_face_3, cyl_dir, full_dz)
full_solid_4 = geompy.MakePrismVecH(full_face_4, cyl_dir, full_dz)

full_internal = geompy.MakeCylinder(full_center, cyl_dir, cyl_radius          , full_dz)
full_external = geompy.MakeCylinder(full_center, cyl_dir, cyl_radius+cyl_thick, full_dz)
full_median   = geompy.MakeCut(full_external, full_internal)

# Boolean operations
# ------------------

full = []

full.append(   geompy.MakeCut(full_solid_1, full_external))
full.append(geompy.MakeCommon(full_solid_1, full_median))

full.append(   geompy.MakeCut(full_solid_2, full_external))
full.append(geompy.MakeCommon(full_solid_2, full_median ))

full.append(   geompy.MakeCut(full_solid_3, full_external))
full.append(geompy.MakeCommon(full_solid_3, full_median))

full.append(   geompy.MakeCut(full_solid_4, full_external))
full.append(geompy.MakeCommon(full_solid_4, full_median))

# Filling the hole
# ----------------

box_d = cyl_radius/3

x = cyl_x-box_d
y = x * cyl_y / cyl_x
box_point_11 = geompy.MakeVertex(x, y, cyl_dz)

x = cyl_x+box_d
y = (box_dx - x) * cyl_y / (box_dx - cyl_x)
box_point_12 = geompy.MakeVertex(x, y, cyl_dz)

x = cyl_x-box_d
y = box_dy - x * (box_dy - cyl_y) / cyl_x
box_point_21 = geompy.MakeVertex(x, y, cyl_dz)

x = cyl_x+box_d
y = box_dy - (box_dx - x) * (box_dy - cyl_y) / (box_dx - cyl_x)
box_point_22 = geompy.MakeVertex(x, y, cyl_dz)

box_face = geompy.MakeQuad4Vertices(box_point_11, box_point_12, box_point_21, box_point_22)

box = geompy.MakePrismVecH(box_face, cyl_dir, full_dz)

full.append(box)

full.append(geompy.MakeCut(geompy.MakeCommon(full_solid_1, full_internal), box))
full.append(geompy.MakeCut(geompy.MakeCommon(full_solid_2, full_internal), box))
full.append(geompy.MakeCut(geompy.MakeCommon(full_solid_3, full_internal), box))
full.append(geompy.MakeCut(geompy.MakeCommon(full_solid_4, full_internal), box))

# Cut the cylinder thikness
# -------------------------

full_plan = geompy.MakePlane(geompy.MakeVertex(0, 0, cyl_dz+cyl_thick), cyl_dir, 5000)

full_parts = geompy.MakePartition(full, [full_plan], [], [], geompy.ShapeType["SOLID"])

# Geometry result
# ---------------

blocks.append(full_parts)

piece_cpd = geompy.MakeCompound(blocks)

piece_ok = geompy.RemoveExtraEdges(piece_cpd, doUnionFaces=True)

piece = geompy.MakeGlueFaces(piece_ok, 1.e-3)

piece_id = geompy.addToStudy(piece, "ex13_hole1partial")

# Meshing
# =======

# Create a mesh
# -------------

hexa = smesh.Mesh(piece, "ex13_hole1partial:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(2)

hexa.Quadrangle()

hexa.Hexahedron()

# Local hypothesis
# ----------------

def local(x, y, z, d):
    edge = geompy.GetEdgeNearPoint(piece, geompy.MakeVertex(x, y, z))
    algo = hexa.Segment(edge)
    algo.NumberOfSegments(d)
    algo.Propagation()

local(0, 0, 100, 40)
local(0, 0, 700, 15)

local(100, 0, 0, 20)
local(0, 100, 0, 20)

local(100, 100, 0, 25)

d = cyl_radius-3*cyl_thick
local(cyl_x+d, cyl_y+d, box_dz, 10)

# Compute the mesh
# ----------------

hexa.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
