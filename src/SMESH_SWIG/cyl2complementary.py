# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

# Piece
# -----

# Create the hexahedrical block geometry of a holed parallelepipede.
# The hole has a T form composed by 2 cylinders with different radius, and their axis are normal.
# This piece is meshed in hexahedrical.

gx = 0
gy = 0
gz = 0

g_dx = 250
g_dy = 200
g_dz = 150

g_rayonGrand = 70
g_rayonPetit = 50

g_trim = 1000

# Geometry
# ========

# The parallelepipede
# -------------------

p_boite = MakeBox(gx-g_dx, gy-g_dy, gz-g_dz,  gx+g_dx, gy+g_dy, gz+g_dz)

# The great cylinder
# ------------------

g_base = MakeVertex(gx-g_dx, gy, gz)
g_dir  = MakeVectorDXDYDZ(1, 0, 0)
g_cyl  = MakeCylinder(g_base, g_dir, g_rayonGrand, g_dx*2)

# The first hole
# --------------

b_boite = MakeCut(p_boite , g_cyl)

# Partitioning
# ------------

p_base = MakeVertex(gx, gy, gz)

p_tools = []

p_tools.append(MakePlane(p_base, MakeVectorDXDYDZ(0,  1   , 0   ), g_trim))
p_tools.append(MakePlane(p_base, MakeVectorDXDYDZ(0,  g_dz, g_dy), g_trim))
p_tools.append(MakePlane(p_base, MakeVectorDXDYDZ(0, -g_dz, g_dy), g_trim))

p_tools.append(MakePlane(MakeVertex(gx-g_rayonPetit, gy, gz), g_dir, g_trim))
p_tools.append(MakePlane(MakeVertex(gx+g_rayonPetit, gy, gz), g_dir, g_trim))

p_piece = MakePartition([b_boite], p_tools, [], [], ShapeType["SOLID"])

# The small cylinder
# ------------------

c_cyl = MakeCylinder(p_base, MakeVectorDXDYDZ(0, 0, 1), g_rayonPetit, g_dz)

# The second hole
# ---------------

d_element = SubShapeAllSorted(p_piece, ShapeType["SOLID"])

d_element[ 8] = MakeCut(d_element[ 8], c_cyl)
d_element[10] = MakeCut(d_element[10], c_cyl)

# Compound
# --------

comp_all = MakeCompound(d_element)
piece = BlocksOp.RemoveExtraEdges(comp_all)
#piece = MakeCompound(d_element)

# Add piece in study
# ------------------

piece_id = addToStudy(piece, "BoxHoled2Cylinders")

# Meshing
# =======

# Mesh with hexahedrons
# ---------------------

m_hexa=MeshHexa(piece, 4, "BoxHoled2CylindersHexa")

# Compute mesh
# ------------

m_hexa.Compute()
