# CEA/LGLS 2004-2005, Francis KLOSS (OCC)
# =======================================

from geompy import *

import smesh

# Geometry
# ========

# A holed cube build by partitioning geometric operations

# Values
# ------

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

b_boite = MakeBox(g_x-g_longueur, g_y-g_hauteur, g_z-g_largeur,  g_x+g_longueur, g_y+g_hauteur, g_z+g_largeur)

# Cylinder
# --------

c_axe = MakeVectorDXDYDZ(0, 1, 0)

c_cyl = MakeCylinder(MakeVertex(g_x, g_y-g_hauteur, g_z), c_axe, g_rayon, g_hauteur*2)

c_piece = MakeCut(b_boite, c_cyl)

# Partition and reperation
# ------------------------

p_centre = MakeVertex(g_x, g_y, g_z)

p_tools = []
p_tools.append(MakePlane(p_centre, MakeVectorDXDYDZ( g_largeur, 0, g_longueur), g_trim))
p_tools.append(MakePlane(p_centre, MakeVectorDXDYDZ(-g_largeur, 0, g_longueur), g_trim))

p_part = MakePartition([c_piece], p_tools, [], [], ShapeType["SOLID"])

p_blocs = RemoveExtraEdges(p_part)
piece   = MakeGlueFaces(p_blocs, 1.e-5)

# Add in study
# ------------

piece_id = addToStudy(piece, "ex07_hole1partition")

# Meshing
# =======

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex07_hole1partition:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(20)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()
