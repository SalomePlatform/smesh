# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

# Piece
# -----

# grid compound of 17 x 17 elements
# an element is compound of 3 cylinders concentriques
# an element is centered in a square of the grid
# prisme the grid

# Geometry
# --------

g_x = 0
g_y = 0
g_z = 0

g_arete   = 50
g_hauteur = 30

g_rayon1 = 20
g_rayon2 = 30
g_rayon3 = 40

# The real value for CEA, but need 3 days for computing
#g_grid = 17
g_grid = 3

g_trim = 1000

# Solids
# ------

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

# Compound
# --------

c_cpd = SubShapeAll(p_partie, ShapeType["SOLID"])
c_cpd.append(s_blo4)

c_element = MakeCompound(c_cpd)

# Grid
# ----

piece = MakeMultiTranslation2D(c_element, MakeVectorDXDYDZ(1, 0, 0), 2*g_arete, g_grid,
                                          MakeVectorDXDYDZ(0, 0, 1), 2*g_arete, g_grid)

# Add in study
# ------------

piece_id = addToStudy(piece, "Grid17partition")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

m_hexa=MeshHexa(piece, 4, "Grid17partitionHexa")

# Compute
# -------

m_hexa.Compute()
