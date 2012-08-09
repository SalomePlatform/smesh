#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

# =======================================
#
from geompy import *

import smesh

# Geometry
# ========

# Create the hexahedrical block geometry of a holed parallelepipede.
# The hole has a T form composed by 2 cylinders with different radius, and their axis are normal.
# This piece is meshed in hexahedrical.

# Values
# ------

gx = 0
gy = 0
gz = 0

g_dx = 250
g_dy = 200
g_dz = 150

g_rayonGrand = 70
g_rayonPetit = 50

g_trim = 1000

# The parallelepipede
# -------------------

p_boite = MakeBox(gx-g_dx, gy-g_dy, gz-g_dz,  gx+g_dx, gy+g_dy, gz+g_dz)

# The great cylinder
# ------------------

g_base = MakeVertex(gx-g_dx, gy, gz)
g_dir  = MakeVectorDXDYDZ(1, 0, 0)
g_cyl  = MakeCylinder(g_base, g_dir, g_rayonGrand, g_dx*2)

# The first hole
# --------------

b_boite = MakeCut(p_boite , g_cyl)

# Partitioning
# ------------

p_base = MakeVertex(gx, gy, gz)

p_tools = []

p_tools.append(MakePlane(p_base, MakeVectorDXDYDZ(0,  1   , 0   ), g_trim))
p_tools.append(MakePlane(p_base, MakeVectorDXDYDZ(0,  g_dz, g_dy), g_trim))
p_tools.append(MakePlane(p_base, MakeVectorDXDYDZ(0, -g_dz, g_dy), g_trim))

p_tools.append(MakePlane(MakeVertex(gx-g_rayonPetit, gy, gz), g_dir, g_trim))
p_tools.append(MakePlane(MakeVertex(gx+g_rayonPetit, gy, gz), g_dir, g_trim))

p_piece = MakePartition([b_boite], p_tools, [], [], ShapeType["SOLID"])

# The small cylinder
# ------------------

c_cyl = MakeCylinder(p_base, MakeVectorDXDYDZ(0, 0, 1), g_rayonPetit, g_dz)

# The second hole
# ---------------

d_element = SubShapeAllSorted(p_piece, ShapeType["SOLID"])

d_element[ 8] = MakeCut(d_element[ 8], c_cyl)
d_element[10] = MakeCut(d_element[10], c_cyl)

# Compound
# --------

piece = RemoveExtraEdges(MakeCompound(d_element))
piece = MakeGlueFaces(piece, 1e-07)

# Add piece in study
# ------------------

piece_id = addToStudy(piece, "ex16_cyl2complementary")

# Meshing
# =======

smesh.SetCurrentStudy(salome.myStudy)

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex16_cyl2complementary:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(12)

hexa.Quadrangle()

hexa.Hexahedron()

# Define local hypothesis
# -----------------------

def local(x, y, z, d):
    edge = GetEdgeNearPoint(piece, MakeVertex(x, y, z))
    algo = hexa.Segment(edge)
    algo.NumberOfSegments(d)
    algo.Propagation()

local(gx     , gy+g_dy, gz+g_dz, 7)
local(gx+g_dx, gy+g_dy, gz     , 21)
local(gx+g_dx, gy-g_dy, gz     , 21)

# Mesh calculus
# -------------

hexa.Compute()
