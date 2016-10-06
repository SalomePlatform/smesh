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

# A holed cube build by partitioning geometric operations

# Values
# ------

g_x = 0
g_y = 0
g_z = 0

g_longueur = 50.0
g_largeur  = 40.0
g_hauteur  = 25.0

g_rayon = 10

g_trim = 1000

# Box
# ---

b_boite = geompy.MakeBox(g_x-g_longueur, g_y-g_hauteur, g_z-g_largeur,  g_x+g_longueur, g_y+g_hauteur, g_z+g_largeur)

# Cylinder
# --------

c_axe = geompy.MakeVectorDXDYDZ(0, 1, 0)

c_cyl = geompy.MakeCylinder(geompy.MakeVertex(g_x, g_y-g_hauteur, g_z), c_axe, g_rayon, g_hauteur*2)

c_piece = geompy.MakeCut(b_boite, c_cyl)

# Partition and reperation
# ------------------------

p_centre = geompy.MakeVertex(g_x, g_y, g_z)

p_tools = []
p_tools.append(geompy.MakePlane(p_centre, geompy.MakeVectorDXDYDZ( g_largeur, 0, g_longueur), g_trim))
p_tools.append(geompy.MakePlane(p_centre, geompy.MakeVectorDXDYDZ(-g_largeur, 0, g_longueur), g_trim))

p_part = geompy.MakePartition([c_piece], p_tools, [], [], geompy.ShapeType["SOLID"])

p_blocs = geompy.RemoveExtraEdges(p_part, doUnionFaces=True)
piece   = geompy.MakeGlueFaces(p_blocs, 1.e-5)

# Add in study
# ------------

piece_id = geompy.addToStudy(piece, "ex07_hole1partition")

# Meshing
# =======

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex07_hole1partition:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(20)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
