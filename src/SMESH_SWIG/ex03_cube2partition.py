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

v_1 = geompy.MakeVertex(g_ox         , g_oy         , g_oz         )
v_2 = geompy.MakeVertex(g_ox+v_arete3, g_oy+g_arete , g_oz+v_arete3)

v_3 = geompy.MakeVertex(g_ox+g_arete , g_oy+g_arete , g_oz+g_arete )
v_4 = geompy.MakeVertex(g_ox+v_arete2, g_oy+v_arete2, g_oz+v_arete2)

# Solids
# ------

s_base = geompy.MakeBoxTwoPnt(v_1, v_2)
s_haut = geompy.MakeBoxTwoPnt(v_3, v_4)

# Partition
# ---------

p_dir1 = geompy.MakeVectorDXDYDZ(1, 0, 0)
p_dir2 = geompy.MakeVectorDXDYDZ(0, 0, 1)
p_dir3 = geompy.MakeVectorDXDYDZ(0, 1, 0)

p_tools = []

p_tools.append(geompy.MakePlane(v_3, p_dir1, g_trim))
p_tools.append(geompy.MakePlane(v_4, p_dir1, g_trim))
p_tools.append(geompy.MakePlane(v_3, p_dir2, g_trim))
p_tools.append(geompy.MakePlane(v_4, p_dir2, g_trim))
p_tools.append(geompy.MakePlane(v_3, p_dir3, g_trim))

piece = geompy.MakePartition([s_base, s_haut], p_tools, [], [], geompy.ShapeType["SOLID"])

# Study
# -----

piece_id = geompy.addToStudy(piece, "ex03_cube2partition")

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

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
