# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

import math

# Piece
# -----

# grid compound by a square with a cylinder on each vertex

# Geometry
# --------

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

# Compound
# --------

c_l = []
c_l.append(pieceSolid1)
c_l.append(pieceSolid2)
c_l.append(pieceSolid3)

c_cpd = MakeCompound(c_l)
piece = MakeGlueFaces(c_cpd, 1.e-5)

# Add in study
# ------------

piece_id = addToStudy(piece, "Grid4pyGibi")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

m_hexa=MeshHexa(piece, 4, "Grid4pyGibiHexa")

# Compute
# -------

m_hexa.Compute()
