# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

# Piece
# -----

# A small cube centered and put on a great cube

# Geometry
# --------

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

p_tools = []
p_tools.append(MakePlane(v_3, p_dir1, g_trim))
p_tools.append(MakePlane(v_4, p_dir1, g_trim))
p_tools.append(MakePlane(v_3, p_dir2, g_trim))
p_tools.append(MakePlane(v_4, p_dir2, g_trim))

p_element = MakePartition([s_base], p_tools, [], [], ShapeType["SOLID"])

# Compound
# --------

c_element = SubShapeAll(p_element, ShapeType["SOLID"])
c_element.append(s_haut)

c_cpd = MakeCompound(c_element)
piece = MakeGlueFaces(c_cpd, 1.e-5)

# Study
# -----

piece_id = addToStudy(piece, "Cubes2partition")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

m_hexa=MeshHexa(piece, 4, "Cubes2partitionHexa")

# Compute
# -------

m_hexa.Compute()
