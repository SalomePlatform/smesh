# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

# Geometry
# ========

# Piece
# -----

# A small cube centered and put on a great cube

ox = 0
oy = 0
oz = 0

arete = 10

# Points
# ------

blockPoint111 = MakeVertex(ox      , oy, oz)
blockPoint211 = MakeVertex(ox+arete, oy, oz)
blockPoint112 = MakeVertex(ox      , oy, oz+arete)
blockPoint212 = MakeVertex(ox+arete, oy, oz+arete)

# Faces
# -----

blockFace1 = MakeQuad4Vertices(blockPoint111, blockPoint211, blockPoint212, blockPoint112)

# Solids
# ------

blockSolid11  = MakePrismVecH(blockFace1, MakeVectorDXDYDZ(0, 1, 0), arete)

# Translations
# ------------

blockSolid21  = MakeTranslation(blockSolid11, arete, 0, 0)
blockSolid31  = MakeTranslation(blockSolid21, arete, 0, 0)

blockSolid12  = MakeTranslation(blockSolid11, 0, 0, arete)
blockSolid22  = MakeTranslation(blockSolid12, arete, 0, 0)
blockSolid32  = MakeTranslation(blockSolid22, arete, 0, 0)

blockSolid13  = MakeTranslation(blockSolid12, 0, 0, arete)
blockSolid23  = MakeTranslation(blockSolid13, arete, 0, 0)
blockSolid33  = MakeTranslation(blockSolid23, arete, 0, 0)

blockSolid111 = MakeTranslation(blockSolid22, 0, arete, 0)

# Compound
# --------

c_l = []
c_l.append(blockSolid11)
c_l.append(blockSolid21)
c_l.append(blockSolid31)
c_l.append(blockSolid12)
c_l.append(blockSolid22)
c_l.append(blockSolid32)
c_l.append(blockSolid13)
c_l.append(blockSolid23)
c_l.append(blockSolid33)
c_l.append(blockSolid111)

c_cpd = MakeCompound(c_l)
piece = MakeGlueFaces(c_cpd, 1.e-5)

# Add in study
# ------------

piece_id = addToStudy(piece, "Cubes2geometry")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

m_hexa=MeshHexa(piece, 4, "Cubes2geometryHexa")

# Compute
# -------

m_hexa.Compute()
