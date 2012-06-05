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

# ==================================
#
from geompy import *

import smesh

# Geometry
# ========

# an assembly of a box, a cylinder and a truncated cone meshed with tetrahedral.

# Define values
# -------------

name = "ex21_lamp"

cote = 60

section = 20
size    = 200

radius_1 = 80
radius_2 = 40
height   = 100

# Build a box
# -----------

box  = MakeBox(-cote, -cote, -cote,  +cote, +cote, +cote)

# Build a cylinder
# ----------------

pt1 = MakeVertex(0, 0, cote/3)
di1 = MakeVectorDXDYDZ(0, 0, 1)
cyl = MakeCylinder(pt1, di1, section, size)

# Build a truncated cone
# ----------------------

pt2 = MakeVertex(0, 0, size)
cone = MakeCone(pt2, di1, radius_1, radius_2, height)

# Fuse
# ----

box_cyl = MakeFuse(box, cyl)
piece = MakeFuse(box_cyl, cone)

# Add in study
# ------------

addToStudy(piece, name)

# Create a group of faces
# -----------------------

group = CreateGroup(piece, ShapeType["FACE"])

group_name = name + "_grp"
addToStudy(group, group_name)
group.SetName(group_name)

# Add faces in the group
# ----------------------

faces = SubShapeAllIDs(piece, ShapeType["FACE"])

UnionIDs(group, faces)

# Create a mesh
# =============

smesh.SetCurrentStudy(salome.myStudy)

# Define a mesh on a geometry
# ---------------------------

tetra = smesh.Mesh(piece, name)

# Define 1D hypothesis
# --------------------

algo1d = tetra.Segment()
algo1d.LocalLength(10)

# Define 2D hypothesis
# --------------------

algo2d = tetra.Triangle()
algo2d.LengthFromEdges()

# Define 3D hypothesis
# --------------------

algo3d = tetra.Tetrahedron(smesh.NETGEN)
algo3d.MaxElementVolume(100)

# Compute the mesh
# ----------------

tetra.Compute()

# Create a groupe of faces
# ------------------------

tetra.Group(group)
