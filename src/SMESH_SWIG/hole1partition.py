# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

import math

# Piece
# -----

# A holed cube at center for partitioning

# Geometry
# --------

g_x = 0
g_y = 0
g_z = 0

g_longueur = 50.0
g_largeur  = 40.0
g_hauteur  = 25.0

g_rayon = 10

g_trim = 1000

# Box
# ---

b_boite = MakeBox(g_x-g_longueur, g_y-g_hauteur, g_z-g_largeur,
                  g_x+g_longueur, g_y+g_hauteur, g_z+g_largeur)

# Cylinder
# --------

c_axe = MakeVectorDXDYDZ(0, 1, 0)

c_cyl0 = MakeCylinder(MakeVertex(g_x, g_y-g_hauteur, g_z), c_axe, g_rayon, g_hauteur*2)
c_cyl  = MakeRotation(c_cyl0, c_axe, math.atan(g_longueur/g_largeur))

c_piece = MakeCut(b_boite, c_cyl)

# Partition
# ---------

p_centre = MakeVertex(g_x, g_y, g_z)

p_tools = []
p_tools.append(MakePlane(p_centre, MakeVectorDXDYDZ( g_largeur, 0, g_longueur), g_trim))
p_tools.append(MakePlane(p_centre, MakeVectorDXDYDZ(-g_largeur, 0, g_longueur), g_trim))

piece = MakePartition([c_piece], p_tools, [], [], ShapeType["SOLID"])

# Add in study
# ------------

piece_id = addToStudy(piece, "Hole1partition")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

m_hexa=MeshHexa(piece, 4, "Hole1partitionHexa")

# Compute
# -------

m_hexa.Compute()
