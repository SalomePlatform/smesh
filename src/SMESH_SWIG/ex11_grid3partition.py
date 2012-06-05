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

# grid compound of 3 x 3 elements
# an element is compound of 3 cylinders concentriques
# an element is centered in a square of the grid
# the smaller cylinder is a hole

# prism the grid, and mesh it in hexahedral way

# Values
# ------

g_x = 0
g_y = 0
g_z = 0

g_arete   = 50
g_hauteur = 30

g_rayon1 = 20
g_rayon2 = 30
g_rayon3 = 40

g_grid = 3

g_trim = 1000

# Element
# -------

e_boite = MakeBox(g_x-g_arete, g_y-g_hauteur, g_z-g_arete,  g_x+g_arete, g_y+g_hauteur, g_z+g_arete)

e_hauteur = 2*g_hauteur
e_centre  = MakeVertex(g_x, g_y-g_hauteur, g_z)
e_dir     = MakeVectorDXDYDZ(0, 1, 0)

e_cyl1 = MakeCylinder(e_centre, e_dir, g_rayon3, e_hauteur)

e_blo1 = MakeCut(e_boite, e_cyl1)

e_cyl2 = MakeCylinder(e_centre, e_dir, g_rayon2, e_hauteur)

e_blo2 = MakeCut(e_cyl1, e_cyl2)

e_cyl3 = MakeCylinder(e_centre, e_dir, g_rayon1, e_hauteur)

e_blo3 = MakeCut(e_cyl2, e_cyl3)

# Partition and repair
# --------------------

p_tools = []
p_tools.append(MakePlane(e_centre, MakeVectorDXDYDZ( 1, 0, 1), g_trim))
p_tools.append(MakePlane(e_centre, MakeVectorDXDYDZ(-1, 0, 1), g_trim))

p_part = MakePartition([e_blo1, e_blo2, e_blo3], p_tools, [], [], ShapeType["SOLID"])

p_element = RemoveExtraEdges(p_part, doUnionFaces=True)

# Grid and glue
# -------------

grid = MakeMultiTranslation2D(p_element, MakeVectorDXDYDZ(1, 0, 0), 2*g_arete, g_grid, MakeVectorDXDYDZ(0, 0, 1), 2*g_arete, g_grid)

piece = MakeGlueFaces(grid, 1e-5)

# Add in study
# ------------

piece_id = addToStudy(piece, "ex11_grid3partition")

# Meshing
# =======

smesh.SetCurrentStudy(salome.myStudy)

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex11_grid3partition:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(3)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()
