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

# Element of a grid compound by a square with a cylinder on each vertex build by using partition

# Values
# ------

ox = 0
oy = 0
oz = 0

arete   =  50
hauteur = 100
rayon   =  10

g_trim = 1000

# Box
# ---

piecePoint = geompy.MakeVertex(ox, oy, oz)

pieceBox = geompy.MakeBoxTwoPnt(piecePoint, geompy.MakeVertex(ox+arete, oy+hauteur, oz+arete))

# Cut by cylinders
# ----------------

dirUp = geompy.MakeVectorDXDYDZ(0, 1, 0)

pieceCut1 = geompy.MakeCut(pieceBox , geompy.MakeCylinder(piecePoint                        , dirUp, rayon, hauteur))
pieceCut2 = geompy.MakeCut(pieceCut1, geompy.MakeCylinder(geompy.MakeVertex(ox+arete, oy, oz      ), dirUp, rayon, hauteur))
pieceCut3 = geompy.MakeCut(pieceCut2, geompy.MakeCylinder(geompy.MakeVertex(ox      , oy, oz+arete), dirUp, rayon, hauteur))
pieceCut4 = geompy.MakeCut(pieceCut3, geompy.MakeCylinder(geompy.MakeVertex(ox+arete, oy, oz+arete), dirUp, rayon, hauteur))

# Compound by make a partition of a solid
# ---------------------------------------

dir = geompy.MakeVectorDXDYDZ(-1, 0, 1)

tools = []
tools.append(geompy.MakePlane(geompy.MakeVertex(ox+rayon, oy, oz      ), dir, g_trim))
tools.append(geompy.MakePlane(geompy.MakeVertex(ox      , oy, oz+rayon), dir, g_trim))

piece = geompy.MakePartition([pieceCut4], tools, [], [], geompy.ShapeType["SOLID"])

# Add in study
# ------------

piece_id = geompy.addToStudy(piece, "ex10_grid4geometry")

# Meshing
# =======

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex10_grid4geometry:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(10)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
