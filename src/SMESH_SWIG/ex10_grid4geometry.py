# CEA/LGLS 2004-2005, Francis KLOSS (OCC)
# =======================================

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
