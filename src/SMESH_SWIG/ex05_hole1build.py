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

# A centered holed cube build by  build by points, edges, faces and solids

# Values
# ------

ox = 0
oy = 0
oz = 0

longueur = 100
largeur  =  80
hauteur  =  50

rayon = 10

# Points
# ------

basePoint111 = MakeVertex(ox         ,  oy, oz)
basePoint211 = MakeVertex(ox+longueur,  oy, oz)
basePoint112 = MakeVertex(ox         ,  oy, oz+largeur)
basePoint212 = MakeVertex(ox+longueur,  oy, oz+largeur)

cx = ox+longueur/2
cy = oy
cz = oz+largeur/2

ll = longueur/largeur
ll = ll*ll
dx = rayon/math.sqrt(1+ll)
dz = rayon/math.sqrt(1+1/ll)

circlePoint1 = MakeVertex(cx-dx, cy, cz-dz)
circlePoint2 = MakeVertex(cx+dx, cy, cz-dz)
circlePoint3 = MakeVertex(cx+dx, cy, cz+dz)
circlePoint4 = MakeVertex(cx-dx, cy, cz+dz)

# Edges
# -----

squareEdge1 = MakeEdge(basePoint111, basePoint211)
squareEdge2 = MakeEdge(basePoint211, basePoint212)
squareEdge3 = MakeEdge(basePoint212, basePoint112)
squareEdge4 = MakeEdge(basePoint112, basePoint111)

diagEdge1   = MakeEdge(basePoint111, circlePoint1)
diagEdge2   = MakeEdge(basePoint211, circlePoint2)
diagEdge3   = MakeEdge(basePoint212, circlePoint3)
diagEdge4   = MakeEdge(basePoint112, circlePoint4)

arcEdge1    = MakeArc(circlePoint1, MakeVertex(cx      , cy, cz-rayon), circlePoint2)
arcEdge2    = MakeArc(circlePoint2, MakeVertex(cx+rayon, cy, cz      ), circlePoint3)
arcEdge3    = MakeArc(circlePoint3, MakeVertex(cx      , cy, cz+rayon), circlePoint4)
arcEdge4    = MakeArc(circlePoint4, MakeVertex(cx-rayon, cy, cz      ), circlePoint1)

# Faces
# -----

baseFace1 = MakeQuad(squareEdge1, diagEdge2, arcEdge1, diagEdge1)
baseFace2 = MakeQuad(squareEdge2, diagEdge3, arcEdge2, diagEdge2)
baseFace3 = MakeQuad(squareEdge3, diagEdge4, arcEdge3, diagEdge3)
baseFace4 = MakeQuad(squareEdge4, diagEdge1, arcEdge4, diagEdge4)

# Solids
# ------

baseVector = MakeVectorDXDYDZ(0, 1, 0)

baseSolid1 = MakePrismVecH(baseFace1, baseVector, hauteur)
baseSolid2 = MakePrismVecH(baseFace2, baseVector, hauteur)
baseSolid3 = MakePrismVecH(baseFace3, baseVector, hauteur)
baseSolid4 = MakePrismVecH(baseFace4, baseVector, hauteur)

# Compound
# --------

c_l = []
c_l.append(baseSolid1)
c_l.append(baseSolid2)
c_l.append(baseSolid3)
c_l.append(baseSolid4)

c_cpd = MakeCompound(c_l)
piece = MakeGlueFaces(c_cpd, 1.e-5)

# Add in study
# ------------

piece_id = addToStudy(piece, "ex05_hole1build")

# Meshing
# =======

smesh.SetCurrentStudy(salome.myStudy)

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex05_hole1build:hexa")

algo = hexa.Segment()

algo.NumberOfSegments(6, 3)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()
