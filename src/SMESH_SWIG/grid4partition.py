# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

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

piece_id = addToStudy(piece, "Grid4partition")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

m_hexa=MeshHexa(piece, 4, "Grid4partitionHexa")

# Compute
# -------

m_hexa.Compute()
