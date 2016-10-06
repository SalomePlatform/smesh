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

e_boite = geompy.MakeBox(g_x-g_arete, g_y-g_hauteur, g_z-g_arete,  g_x+g_arete, g_y+g_hauteur, g_z+g_arete)

e_hauteur = 2*g_hauteur
e_centre  = geompy.MakeVertex(g_x, g_y-g_hauteur, g_z)
e_dir     = geompy.MakeVectorDXDYDZ(0, 1, 0)

e_cyl1 = geompy.MakeCylinder(e_centre, e_dir, g_rayon3, e_hauteur)

e_blo1 = geompy.MakeCut(e_boite, e_cyl1)

e_cyl2 = geompy.MakeCylinder(e_centre, e_dir, g_rayon2, e_hauteur)

e_blo2 = geompy.MakeCut(e_cyl1, e_cyl2)

e_cyl3 = geompy.MakeCylinder(e_centre, e_dir, g_rayon1, e_hauteur)

e_blo3 = geompy.MakeCut(e_cyl2, e_cyl3)

# Partition and repair
# --------------------

p_tools = []
p_tools.append(geompy.MakePlane(e_centre, geompy.MakeVectorDXDYDZ( 1, 0, 1), g_trim))
p_tools.append(geompy.MakePlane(e_centre, geompy.MakeVectorDXDYDZ(-1, 0, 1), g_trim))

p_part = geompy.MakePartition([e_blo1, e_blo2, e_blo3], p_tools, [], [], geompy.ShapeType["SOLID"])

p_element = geompy.RemoveExtraEdges(p_part, doUnionFaces=True)

# Grid and glue
# -------------

grid = geompy.MakeMultiTranslation2D(p_element, geompy.MakeVectorDXDYDZ(1, 0, 0), 2*g_arete, g_grid, geompy.MakeVectorDXDYDZ(0, 0, 1), 2*g_arete, g_grid)

piece = geompy.MakeGlueFaces(grid, 1e-5)

# Add in study
# ------------

piece_id = geompy.addToStudy(piece, "ex11_grid3partition")

# Meshing
# =======

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

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
