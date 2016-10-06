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

# =======================================
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

p_boite = geompy.MakeBox(gx-g_dx, gy-g_dy, gz-g_dz,  gx+g_dx, gy+g_dy, gz+g_dz)

# The great cylinder
# ------------------

g_base = geompy.MakeVertex(gx-g_dx, gy, gz)
g_dir  = geompy.MakeVectorDXDYDZ(1, 0, 0)
g_cyl  = geompy.MakeCylinder(g_base, g_dir, g_rayonGrand, g_dx*2)

# The first hole
# --------------

b_boite = geompy.MakeCut(p_boite , g_cyl)

# Partitioning
# ------------

p_base = geompy.MakeVertex(gx, gy, gz)

p_tools = []

p_tools.append(geompy.MakePlane(p_base, geompy.MakeVectorDXDYDZ(0,  1   , 0   ), g_trim))
p_tools.append(geompy.MakePlane(p_base, geompy.MakeVectorDXDYDZ(0,  g_dz, g_dy), g_trim))
p_tools.append(geompy.MakePlane(p_base, geompy.MakeVectorDXDYDZ(0, -g_dz, g_dy), g_trim))

p_tools.append(geompy.MakePlane(geompy.MakeVertex(gx-g_rayonPetit, gy, gz), g_dir, g_trim))
p_tools.append(geompy.MakePlane(geompy.MakeVertex(gx+g_rayonPetit, gy, gz), g_dir, g_trim))

p_piece = geompy.MakePartition([b_boite], p_tools, [], [], geompy.ShapeType["SOLID"])

# The small cylinder
# ------------------

c_cyl = geompy.MakeCylinder(p_base, geompy.MakeVectorDXDYDZ(0, 0, 1), g_rayonPetit, g_dz)

# The second hole
# ---------------

d_element = geompy.SubShapeAllSorted(p_piece, geompy.ShapeType["SOLID"])

d_element[ 8] = geompy.MakeCut(d_element[ 8], c_cyl)
d_element[10] = geompy.MakeCut(d_element[10], c_cyl)

# Compound
# --------

piece = geompy.RemoveExtraEdges(geompy.MakeCompound(d_element))
piece = geompy.MakeGlueFaces(piece, 1e-07)

# Add piece in study
# ------------------

piece_id = geompy.addToStudy(piece, "ex16_cyl2complementary")

# Meshing
# =======

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
    edge = geompy.GetEdgeNearPoint(piece, geompy.MakeVertex(x, y, z))
    algo = hexa.Segment(edge)
    algo.NumberOfSegments(d)
    algo.Propagation()

local(gx     , gy+g_dy, gz+g_dz, 7)
local(gx+g_dx, gy+g_dy, gz     , 21)
local(gx+g_dx, gy-g_dy, gz     , 21)

# Mesh calculus
# -------------

hexa.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
