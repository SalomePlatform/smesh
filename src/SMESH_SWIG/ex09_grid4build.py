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

import math

# Geometry
# ========

# Element of a grid compound by a square with a cylinder on each vertex build by points, edges, faces and solids

# Values
# ------

ox = 0
oy = 0
oz = 0

arete   =  50
hauteur = 100

rayon = 10

demi = rayon/2
r3   = demi*math.sqrt(3)

# Points
# ------

piecePoint111 = MakeVertex(ox+rayon      , oy, oz)
piecePoint211 = MakeVertex(ox+arete-rayon, oy, oz)
piecePoint112 = MakeVertex(ox            , oy, oz+rayon)
piecePoint212 = MakeVertex(ox+arete      , oy, oz+rayon)
piecePoint113 = MakeVertex(ox            , oy, oz+arete-rayon)
piecePoint213 = MakeVertex(ox+arete      , oy, oz+arete-rayon)
piecePoint114 = MakeVertex(ox+rayon      , oy, oz+arete)
piecePoint214 = MakeVertex(ox+arete-rayon, oy, oz+arete)

pieceCenter1  = MakeVertex(ox            , oy, oz)
pieceCenter2  = MakeVertex(ox+arete      , oy, oz)
pieceCenter3  = MakeVertex(ox            , oy, oz+arete)
pieceCenter4  = MakeVertex(ox+arete      , oy, oz+arete)

piecePass1    = MakeVertex(ox+demi       , oy, oz+r3)
piecePass2    = MakeVertex(ox+arete-demi , oy, oz+r3)
piecePass3    = MakeVertex(ox+arete-demi , oy, oz+arete-r3)
piecePass4    = MakeVertex(ox+demi       , oy, oz+arete-r3)

# Edges
# -----

pieceEdgeSquare1   = MakeEdge(piecePoint111, piecePoint211)
pieceEdgeSquare2   = MakeEdge(piecePoint114, piecePoint214)
pieceEdgeSquare3   = MakeEdge(piecePoint112, piecePoint113)
pieceEdgeSquare4   = MakeEdge(piecePoint212, piecePoint213)

pieceEdgeDiagonal1 = MakeEdge(piecePoint111, piecePoint213)
pieceEdgeDiagonal2 = MakeEdge(piecePoint112, piecePoint214)

pieceEdgeArc1 = MakeArc(piecePoint111, piecePass1, piecePoint112)
pieceEdgeArc2 = MakeArc(piecePoint211, piecePass2, piecePoint212)
pieceEdgeArc3 = MakeArc(piecePoint213, piecePass3, piecePoint214)
pieceEdgeArc4 = MakeArc(piecePoint113, piecePass4, piecePoint114)

# Faces
# -----

pieceFace1 = MakeQuad(pieceEdgeSquare1, pieceEdgeArc2, pieceEdgeSquare4, pieceEdgeDiagonal1)
pieceFace2 = MakeQuad(pieceEdgeSquare2, pieceEdgeArc4, pieceEdgeSquare3, pieceEdgeDiagonal2)

pieceFace3 = MakeQuad(pieceEdgeArc1, pieceEdgeDiagonal1, pieceEdgeArc3, pieceEdgeDiagonal2)

# Solids
# ------

pieceVector = MakeVectorDXDYDZ(0, 1, 0)

pieceSolid1 = MakePrismVecH(pieceFace1, pieceVector, hauteur)
pieceSolid2 = MakePrismVecH(pieceFace2, pieceVector, hauteur)
pieceSolid3 = MakePrismVecH(pieceFace3, pieceVector, hauteur)

# Compound and glue
# -----------------

c_cpd = MakeCompound([pieceSolid1, pieceSolid2, pieceSolid3])

piece = MakeGlueFaces(c_cpd, 1.e-5)

# Add in study
# ------------

piece_id = addToStudy(piece, "ex09_grid4build")

# Meshing
# =======

smesh.SetCurrentStudy(salome.myStudy)

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex09_grid4build:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(6)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()
