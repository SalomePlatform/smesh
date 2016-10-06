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

# A small cube centered and put on a great cube build by primitive geometric functionalities

# Values
# ------

ox = 0
oy = 0
oz = 0

arete = 10

# Points
# ------

blockPoint111 = geompy.MakeVertex(ox      , oy, oz)
blockPoint211 = geompy.MakeVertex(ox+arete, oy, oz)
blockPoint112 = geompy.MakeVertex(ox      , oy, oz+arete)
blockPoint212 = geompy.MakeVertex(ox+arete, oy, oz+arete)

# Face and solid
# --------------

blockFace1 = geompy.MakeQuad4Vertices(blockPoint111, blockPoint211, blockPoint212, blockPoint112)

blockSolid11  = geompy.MakePrismVecH(blockFace1, geompy.MakeVectorDXDYDZ(0, 1, 0), arete)

# Translations
# ------------

blockSolid21  = geompy.MakeTranslation(blockSolid11, arete, 0, 0)
blockSolid31  = geompy.MakeTranslation(blockSolid21, arete, 0, 0)

blockSolid12  = geompy.MakeTranslation(blockSolid11, 0, 0, arete)
blockSolid22  = geompy.MakeTranslation(blockSolid12, arete, 0, 0)
blockSolid32  = geompy.MakeTranslation(blockSolid22, arete, 0, 0)

blockSolid13  = geompy.MakeTranslation(blockSolid12, 0, 0, arete)
blockSolid23  = geompy.MakeTranslation(blockSolid13, arete, 0, 0)
blockSolid33  = geompy.MakeTranslation(blockSolid23, arete, 0, 0)

blockSolid111 = geompy.MakeTranslation(blockSolid22, 0, arete, 0)

# Compound and glue
# -----------------

c_l = []
c_l.append(blockSolid11)
c_l.append(blockSolid21)
c_l.append(blockSolid31)
c_l.append(blockSolid12)
c_l.append(blockSolid22)
c_l.append(blockSolid32)
c_l.append(blockSolid13)
c_l.append(blockSolid23)
c_l.append(blockSolid33)
c_l.append(blockSolid111)

c_cpd = geompy.MakeCompound(c_l)

piece = geompy.MakeGlueFaces(c_cpd, 1.e-5)

# Add in study
# ------------

piece_id = geompy.addToStudy(piece, "ex02_cube2primitive")

# Meshing
# =======

# Create hexahedrical mesh on piece
# ---------------------------------

hexa = smesh.Mesh(piece, "ex02_cube2primitive:hexa")

algo = hexa.Segment()
algo.LocalLength(1)

hexa.Quadrangle()

hexa.Hexahedron()

# Compute the mesh
# ----------------

hexa.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
