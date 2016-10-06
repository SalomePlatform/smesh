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

# A not centered holed cube build by boolean geometric operations

# Values
# ------

ox = 0
oy = 0
oz = 0

longueur1 = 30
longueur2 = 70

largeur1  = 30
largeur2  = 50

hauteur   = 50

rayon = 10

# Triangular face
# ---------------

def triangle(p1, p2, p3):
    l = []
    l.append(geompy.MakeEdge(p1, p2))
    l.append(geompy.MakeEdge(p2, p3))
    l.append(geompy.MakeEdge(p3, p1))
    w = geompy.MakeWire(l)
    return geompy.MakeFace(w, 1)

# Points
# ------

basePoint111 = geompy.MakeVertex(ox-longueur1,  oy, oz-largeur1)
basePoint211 = geompy.MakeVertex(ox+longueur2,  oy, oz-largeur1)
basePoint112 = geompy.MakeVertex(ox-longueur1,  oy, oz+largeur2)
basePoint212 = geompy.MakeVertex(ox+longueur2,  oy, oz+largeur2)

holePoint    = geompy.MakeVertex(ox, oy, oz)

# Faces
# -----

baseFace1 = triangle(basePoint111, basePoint211, holePoint)
baseFace2 = triangle(basePoint211, basePoint212, holePoint)
baseFace3 = triangle(basePoint212, basePoint112, holePoint)
baseFace4 = triangle(basePoint112, basePoint111, holePoint)

# Solids
# ------

baseVector = geompy.MakeVectorDXDYDZ(0, 1, 0)

baseSolid1 = geompy.MakePrismVecH(baseFace1, baseVector, hauteur)
baseSolid2 = geompy.MakePrismVecH(baseFace2, baseVector, hauteur)
baseSolid3 = geompy.MakePrismVecH(baseFace3, baseVector, hauteur)
baseSolid4 = geompy.MakePrismVecH(baseFace4, baseVector, hauteur)

holeSolid = geompy.MakeCylinder(holePoint, baseVector, rayon, hauteur)

# Boolean operations
# ------------------

baseHexa1 = geompy.MakeCut(baseSolid1, holeSolid)
baseHexa2 = geompy.MakeCut(baseSolid2, holeSolid)
baseHexa3 = geompy.MakeCut(baseSolid3, holeSolid)
baseHexa4 = geompy.MakeCut(baseSolid4, holeSolid)

# Compound, glue and repair
# -------------------------

c_l = []
c_l.append(baseHexa1)
c_l.append(baseHexa2)
c_l.append(baseHexa3)
c_l.append(baseHexa4)

c_cpd = geompy.MakeCompound(c_l)
c_glu = geompy.MakeGlueFaces(c_cpd, 1.e-5)
piece = geompy.RemoveExtraEdges(c_glu, doUnionFaces=True)

# Add in study
# ------------

piece_id = geompy.addToStudy(piece, "ex06_hole1boolean")

# Meshing
# =======

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex06_hole1boolean:hexa")

algo = hexa.Segment()

algo.NumberOfSegments(11)

hexa.Quadrangle()

hexa.Hexahedron()

# Create local hypothesis
# -----------------------

edge1 = geompy.GetEdgeNearPoint(piece, geompy.MakeVertex(ox, oy, oz-largeur1))
algo1 = hexa.Segment(edge1)
algo1.NumberOfSegments(3)
algo1.Propagation()

edge2 = geompy.GetEdgeNearPoint(piece, geompy.MakeVertex(ox-longueur1, oy, oz))
algo2 = hexa.Segment(edge2)
algo2.NumberOfSegments(5)
algo2.Propagation()

edge3 = geompy.GetEdgeNearPoint(piece, geompy.MakeVertex(ox, oy, oz+largeur2))
algo3 = hexa.Segment(edge3)
algo3.NumberOfSegments(7)
algo3.Propagation()

edge4 = geompy.GetEdgeNearPoint(piece, geompy.MakeVertex(ox+longueur2, oy, oz))
algo4 = hexa.Segment(edge4)
algo4.NumberOfSegments(9)
algo4.Propagation()

# Mesh calculus
# -------------

hexa.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
