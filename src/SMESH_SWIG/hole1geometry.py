# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

# Piece
# -----

# A holed cube

# Geometry
# --------

ox = 0
oy = 0
oz = 0

longueur1 = 30
longueur2 = 70

largeur1  = 30
largeur2  = 50

hauteur   = 50

rayon = 10

# Triangular face
# ---------------

def triangle(p1, p2, p3):
    l = []
    l.append(MakeEdge(p1, p2))
    l.append(MakeEdge(p2, p3))
    l.append(MakeEdge(p3, p1))
    w = MakeWire(l)
    return MakeFace(w, 1)

# Points
# ------

basePoint111 = MakeVertex(ox-longueur1,  oy, oz-largeur1)
basePoint211 = MakeVertex(ox+longueur2,  oy, oz-largeur1)
basePoint112 = MakeVertex(ox-longueur1,  oy, oz+largeur2)
basePoint212 = MakeVertex(ox+longueur2,  oy, oz+largeur2)

holePoint    = MakeVertex(ox, oy, oz)

# Faces
# -----

baseFace1 = triangle(basePoint111, basePoint211, holePoint)
baseFace2 = triangle(basePoint211, basePoint212, holePoint)
baseFace3 = triangle(basePoint212, basePoint112, holePoint)
baseFace4 = triangle(basePoint112, basePoint111, holePoint)

# Solids
# ------

baseVector = MakeVectorDXDYDZ(0, 1, 0)

baseSolid1 = MakePrismVecH(baseFace1, baseVector, hauteur)
baseSolid2 = MakePrismVecH(baseFace2, baseVector, hauteur)
baseSolid3 = MakePrismVecH(baseFace3, baseVector, hauteur)
baseSolid4 = MakePrismVecH(baseFace4, baseVector, hauteur)

holeSolid = MakeCylinder(holePoint, baseVector, rayon, hauteur)

# Boolean operations
# ------------------

baseHexa1 = MakeCut(baseSolid1, holeSolid)
baseHexa2 = MakeCut(baseSolid2, holeSolid)
baseHexa3 = MakeCut(baseSolid3, MakeRotation(holeSolid, baseVector, 3.141592653))
baseHexa4 = MakeCut(baseSolid4, holeSolid)

# Compound
# --------

c_l = []
c_l.append(baseHexa1)
c_l.append(baseHexa2)
c_l.append(baseHexa3)
c_l.append(baseHexa4)

c_cpd = MakeCompound(c_l)
piece = MakeGlueFaces(c_cpd, 1.e-5)

# Add in study
# ------------

piece_id = addToStudy(piece, "Hole1geometry")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

m_hexa=MeshHexa(piece, 4, "Hole1geometryHexa")

# Compute
# -------

m_hexa.Compute()
