# CEA/LGLS 2004-2005, Francis KLOSS (OCC)
# =======================================

from geompy import *

import smesh

# Geometry
# ========

# 5 box with a hexahedral mesh and with 2 box in tetrahedral mesh

# Values
# ------

arete = 100

arete0 = 0
arete1 = arete
arete2 = arete*2
arete3 = arete*3

# Solids
# ------

box_tetra1 = MakeBox(arete0, arete0, 0,  arete1, arete1, arete)

box_ijk1   = MakeBox(arete1, arete0, 0,  arete2, arete1, arete)

box_hexa   = MakeBox(arete1, arete1, 0,  arete2, arete2, arete)

box_ijk2   = MakeBox(arete2, arete1, 0,  arete3, arete2, arete)

box_tetra2 = MakeBox(arete2, arete2, 0,  arete3 ,arete3, arete)

# Piece
# -----

piece_cpd = MakeCompound([box_tetra1, box_ijk1, box_hexa, box_ijk2, box_tetra2])

piece = MakeGlueFaces(piece_cpd, 1e-4)

piece_id = addToStudy(piece, "ex04_cube5tetraHexa")

# Meshing
# =======

# Create a hexahedral mesh
# ------------------------

mixed = smesh.Mesh(piece, "ex04_cube5tetraHexa:mixed")

algo = mixed.Segment()

algo.StartEndLength(3, 25)

mixed.Quadrangle()

mixed.Hexahedron()

# Tetrahedral local mesh
# ----------------------

def localMesh(b, hyp):
    box   = GetInPlace(piece, b)
    faces = SubShapeAll(box, ShapeType["FACE"])

    i = 0
    n = len(faces)
    while i<n:
        algo = mixed.Triangle(faces[i])
        if hyp:
            algo.MaxElementArea(80)
        else:
            algo.LengthFromEdges()
        i = i + 1

    algo = mixed.Tetrahedron(smesh.NETGEN, box)
    algo.MaxElementVolume(400)

localMesh(box_tetra1, 1)
localMesh(box_tetra2, 0)

# Mesh calculus
# -------------

mixed.Compute()
