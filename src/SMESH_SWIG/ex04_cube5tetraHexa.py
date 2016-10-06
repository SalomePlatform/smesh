#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# =======================================
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

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

box_tetra1 = geompy.MakeBox(arete0, arete0, 0,  arete1, arete1, arete)

box_ijk1   = geompy.MakeBox(arete1, arete0, 0,  arete2, arete1, arete)

box_hexa   = geompy.MakeBox(arete1, arete1, 0,  arete2, arete2, arete)

box_ijk2   = geompy.MakeBox(arete2, arete1, 0,  arete3, arete2, arete)

box_tetra2 = geompy.MakeBox(arete2, arete2, 0,  arete3 ,arete3, arete)

# Piece
# -----

piece_cpd = geompy.MakeCompound([box_tetra1, box_ijk1, box_hexa, box_ijk2, box_tetra2])

piece = geompy.MakeGlueFaces(piece_cpd, 1e-4)

piece_id = geompy.addToStudy(piece, "ex04_cube5tetraHexa")

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
    box   = geompy.GetInPlace(piece, b)
    faces = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])

    i = 0
    n = len(faces)
    while i<n:
        algo = mixed.Triangle(faces[i])
        if hyp:
            algo.MaxElementArea(80)
        else:
            algo.LengthFromEdges()
        i = i + 1

    algo = mixed.Tetrahedron(smeshBuilder.NETGEN, box)
    algo.MaxElementVolume(400)

localMesh(box_tetra1, 1)
localMesh(box_tetra2, 0)

# Mesh calculus
# -------------

mixed.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
