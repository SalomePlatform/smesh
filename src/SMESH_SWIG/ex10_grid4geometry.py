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

piecePoint = MakeVertex(ox, oy, oz)

pieceBox = MakeBoxTwoPnt(piecePoint, MakeVertex(ox+arete, oy+hauteur, oz+arete))

# Cut by cylinders
# ----------------

dirUp = MakeVectorDXDYDZ(0, 1, 0)

pieceCut1 = MakeCut(pieceBox , MakeCylinder(piecePoint                        , dirUp, rayon, hauteur))
pieceCut2 = MakeCut(pieceCut1, MakeCylinder(MakeVertex(ox+arete, oy, oz      ), dirUp, rayon, hauteur))
pieceCut3 = MakeCut(pieceCut2, MakeCylinder(MakeVertex(ox      , oy, oz+arete), dirUp, rayon, hauteur))
pieceCut4 = MakeCut(pieceCut3, MakeCylinder(MakeVertex(ox+arete, oy, oz+arete), dirUp, rayon, hauteur))

# Compound by make a partition of a solid
# ---------------------------------------

dir = MakeVectorDXDYDZ(-1, 0, 1)

tools = []
tools.append(MakePlane(MakeVertex(ox+rayon, oy, oz      ), dir, g_trim))
tools.append(MakePlane(MakeVertex(ox      , oy, oz+rayon), dir, g_trim))

piece = MakePartition([pieceCut4], tools, [], [], ShapeType["SOLID"])

# Add in study
# ------------

piece_id = addToStudy(piece, "ex10_grid4geometry")

# Meshing
# =======

smesh.SetCurrentStudy(salome.myStudy)

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
