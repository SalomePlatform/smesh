# CEA/LGLS 2004-2005, Francis KLOSS (OCC)
# =======================================

from geompy import *

import smesh

# Geometry
# ========

# A small cube centered and put on a great cube build with partition

# Values
# ------

g_ox = 0
g_oy = 0
g_oz = 0

g_arete = 10

g_trim = 1000

# Points
# ------

v_arete2 = g_arete*2
v_arete3 = g_arete*3

v_1 = MakeVertex(g_ox         , g_oy         , g_oz         )
v_2 = MakeVertex(g_ox+v_arete3, g_oy+g_arete , g_oz+v_arete3)

v_3 = MakeVertex(g_ox+g_arete , g_oy+g_arete , g_oz+g_arete )
v_4 = MakeVertex(g_ox+v_arete2, g_oy+v_arete2, g_oz+v_arete2)

# Solids
# ------

s_base = MakeBoxTwoPnt(v_1, v_2)
s_haut = MakeBoxTwoPnt(v_3, v_4)

# Partition
# ---------

p_dir1 = MakeVectorDXDYDZ(1, 0, 0)
p_dir2 = MakeVectorDXDYDZ(0, 0, 1)
p_dir3 = MakeVectorDXDYDZ(0, 1, 0)

p_tools = []

p_tools.append(MakePlane(v_3, p_dir1, g_trim))
p_tools.append(MakePlane(v_4, p_dir1, g_trim))
p_tools.append(MakePlane(v_3, p_dir2, g_trim))
p_tools.append(MakePlane(v_4, p_dir2, g_trim))
p_tools.append(MakePlane(v_3, p_dir3, g_trim))

piece = MakePartition([s_base, s_haut], p_tools, [], [], ShapeType["SOLID"])

# Study
# -----

piece_id = addToStudy(piece, "ex03_cube2partition")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

hexa = smesh.Mesh(piece, "ex03_cube2partition:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(5)

hexa.Quadrangle()

hexa.Hexahedron()

# Compute the mesh
# ----------------

hexa.Compute()
