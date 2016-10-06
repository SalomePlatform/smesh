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

# A twice holed cube build by points, edges, faces and solids

# Values
# ------

ox = 0
oy = 0
oz = 0

longueur = 200
largeur  = 100
hauteur  =  80

cylindre = 50

rayon = 20

# Points
# ------

piecePoint1 = geompy.MakeVertex(ox         , oy, oz)
piecePoint2 = geompy.MakeVertex(ox+longueur, oy, oz)
piecePoint3 = geompy.MakeVertex(ox+longueur, oy, oz+largeur)
piecePoint4 = geompy.MakeVertex(ox         , oy, oz+largeur)

cz = oz+largeur/2

cylPoint1    = geompy.MakeVertex(ox+cylindre         , oy, cz-rayon)
cylPoint2    = geompy.MakeVertex(ox+longueur-cylindre, oy, cz-rayon)
cylPoint3    = geompy.MakeVertex(ox+longueur-cylindre, oy, cz+rayon)
cylPoint4    = geompy.MakeVertex(ox+cylindre         , oy, cz+rayon)

# Edges
# -----

pieceEdge1 = geompy.MakeEdge(piecePoint1, piecePoint4)
pieceEdge2 = geompy.MakeEdge(piecePoint1, cylPoint1)
pieceEdge3 = geompy.MakeEdge(piecePoint4, cylPoint4)

pieceEdge4 = geompy.MakeEdge(piecePoint2, piecePoint3)
pieceEdge5 = geompy.MakeEdge(piecePoint2, cylPoint2)
pieceEdge6 = geompy.MakeEdge(piecePoint3, cylPoint3)

pieceEdge7 = geompy.MakeEdge(cylPoint1, cylPoint2)
pieceEdge8 = geompy.MakeEdge(cylPoint3, cylPoint4)

cylEdge1 = geompy.MakeArc(cylPoint1, geompy.MakeVertex(ox+cylindre-rayon         , oy, cz), cylPoint4)
cylEdge2 = geompy.MakeArc(cylPoint1, geompy.MakeVertex(ox+cylindre+rayon         , oy, cz), cylPoint4)
cylEdge3 = geompy.MakeArc(cylPoint2, geompy.MakeVertex(ox+longueur-cylindre-rayon, oy, cz), cylPoint3)
cylEdge4 = geompy.MakeArc(cylPoint2, geompy.MakeVertex(ox+longueur-cylindre+rayon, oy, cz), cylPoint3)

# Faces
# -----

pieceFace1 = geompy.MakeQuad4Vertices(piecePoint1, piecePoint2, cylPoint2 , cylPoint1 )
pieceFace2 = geompy.MakeQuad         (pieceEdge1 , pieceEdge2 , cylEdge1  , pieceEdge3)
pieceFace3 = geompy.MakeQuad4Vertices(piecePoint3, piecePoint4, cylPoint4 , cylPoint3 )
pieceFace4 = geompy.MakeQuad         (pieceEdge4 , pieceEdge5 , cylEdge4  , pieceEdge6)
pieceFace5 = geompy.MakeQuad         (pieceEdge7 , cylEdge3   , pieceEdge8, cylEdge2  )

# Solids
# ------

pieceVector = geompy.MakeVectorDXDYDZ(0, 1, 0)

pieceSolid1 = geompy.MakePrismVecH(pieceFace1, pieceVector, hauteur)
pieceSolid2 = geompy.MakePrismVecH(pieceFace2, pieceVector, hauteur)
pieceSolid3 = geompy.MakePrismVecH(pieceFace3, pieceVector, hauteur)
pieceSolid4 = geompy.MakePrismVecH(pieceFace4, pieceVector, hauteur)
pieceSolid5 = geompy.MakePrismVecH(pieceFace5, pieceVector, hauteur)

# Compound and glue
# -----------------

c_cpd = geompy.MakeCompound([pieceSolid1, pieceSolid2, pieceSolid3, pieceSolid4, pieceSolid5])

piece = geompy.MakeGlueFaces(c_cpd, 1.e-5)

# Add in study
# ------------

piece_id = geompy.addToStudy(piece, "ex08_hole2build")

# Meshing
# =======

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex08_hole2build:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(7)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
