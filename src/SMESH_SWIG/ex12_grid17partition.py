# CEA/LGLS 2004-2005, Francis KLOSS (OCC)
# =======================================

from geompy import *

import smesh

# Geometry
# ========

# grid compound of 17 x 17 elements
# an element is compound of 3 concentric cylinders
# an element is centered in a square of the grid

# prism the grid, and mesh it in hexahedral way

# Values
# ------

g_x = 0
g_y = 0
g_z = 0

g_arete   = 50
g_hauteur = 30

g_rayon1 = 20
g_rayon2 = 30
g_rayon3 = 40

g_grid = 17

g_trim = 1000

# Solids and rotation to prevent repair
# -------------------------------------

s_boite = MakeBox(g_x-g_arete, g_y-g_hauteur, g_z-g_arete,  g_x+g_arete, g_y+g_hauteur, g_z+g_arete)

s_pi4     = 3.141592653/4
s_hauteur = 2*g_hauteur
s_centre  = MakeVertex(g_x, g_y-g_hauteur, g_z)
s_dir     = MakeVectorDXDYDZ(0, 1, 0)

s_cyl0 = MakeCylinder(s_centre, s_dir, g_rayon3, s_hauteur)
s_cyl1 = MakeRotation(s_cyl0, s_dir, s_pi4)

s_blo1 = MakeCut(s_boite, s_cyl1)

s_cyl0 = MakeCylinder(s_centre, s_dir, g_rayon2, s_hauteur)
s_cyl2 = MakeRotation(s_cyl0, s_dir, s_pi4)

s_blo2 = MakeCut(s_cyl1, s_cyl2)

s_cyl0 = MakeCylinder(s_centre, s_dir, g_rayon1, s_hauteur)
s_cyl3 = MakeRotation(s_cyl0, s_dir, s_pi4)

s_blo3 = MakeCut(s_cyl2, s_cyl3)

s_arete = g_rayon1/2

s_blo4 = MakeBox(g_x-s_arete, g_y-g_hauteur, g_z-s_arete,  g_x+s_arete, g_y+g_hauteur, g_z+s_arete)

s_blo5 = MakeCut(s_cyl3, s_blo4)

# Partition
# ---------

p_tools = []
p_tools.append(MakePlane(s_centre, MakeVectorDXDYDZ( 1, 0, 1), g_trim))
p_tools.append(MakePlane(s_centre, MakeVectorDXDYDZ(-1, 0, 1), g_trim))

p_partie = MakePartition([s_blo1, s_blo2, s_blo3, s_blo5], p_tools, [], [], ShapeType["SOLID"])

# Compound and glue
# -----------------

c_blocs = SubShapeAll(p_partie, ShapeType["SOLID"])
c_blocs.append(s_blo4)

c_cpd = MakeCompound(c_blocs)

c_element = MakeGlueFaces(c_cpd, 1e-4)

# Grid
# ----

piece = MakeMultiTranslation2D(c_element, MakeVectorDXDYDZ(1, 0, 0), 2*g_arete, g_grid, MakeVectorDXDYDZ(0, 0, 1), 2*g_arete, g_grid)

# Add in study
# ------------

piece_id = addToStudy(piece, "ex12_grid17partition")

# Meshing
# =======

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex12_grid17partition:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(2)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()
