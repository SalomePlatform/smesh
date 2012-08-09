#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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
from geompy import *

import smesh

import math

# Geometrie
# =========

# Creer un cylindre ayant a chaque bout un morceau de sphere et le tout troue par un petit cylindrique excentre.
# Decouper en hexahedre et mailler.

# Donnees
# -------

# unite: millimetre

g_ox = 0
g_oy = 0
g_oz = 0

g_cyl_rayon       = 1000
g_cyl_demiHauteur = 3000

g_trou_rayon       =   5
g_trou_centre      = 300

g_sphere_rayon = 3500

g_trim = 15000

# Cylindre
# --------

c_point    = MakeVertex(g_ox, g_oy, g_oz-g_cyl_demiHauteur)
c_dir      = MakeVectorDXDYDZ(0, 0, 1)
c_hauteur  = 2*g_cyl_demiHauteur

c_cylindre = MakeCylinder(c_point, c_dir, g_cyl_rayon, c_hauteur)

# Sphere
# ------

s_hauteur = math.sqrt(g_sphere_rayon*g_sphere_rayon - g_cyl_rayon*g_cyl_rayon) - g_cyl_demiHauteur

s_sphere  = MakeSphere(g_ox, g_oy, g_oz-s_hauteur, g_sphere_rayon)

# Calottes
# --------

c_outils = []
c_outils.append(MakePlane(MakeVertex(g_ox, g_oy, g_oz+g_cyl_demiHauteur), MakeVectorDXDYDZ(0, 0, 1), g_trim))

c_cpd = MakePartition([s_sphere], c_outils, [], [], ShapeType["SOLID"])
c_calotte_haut, c_reste = SubShapeAllSorted(c_cpd, ShapeType["SOLID"])

c_plan = MakePlane(MakeVertex(g_ox, g_oy, g_oz), MakeVectorDXDYDZ(0, 0, 1), g_trim)
c_calotte_bas = MakeMirrorByPlane(c_calotte_haut, c_plan)

# Fusionner
# ---------

f_piece1 = MakeFuse(c_cylindre, c_calotte_haut)
f_piece  = MakeFuse(f_piece1, c_calotte_bas)

# Trouer
# ------

t_hauteur = g_sphere_rayon
t_point   = MakeVertex(g_ox-g_trou_centre, g_oy, g_oz-t_hauteur)
t_trou    = MakeCylinder(t_point, c_dir, g_trou_rayon, 2*t_hauteur)

t_piece   = MakeCut(f_piece, t_trou)

# Decouper
# --------

h_outils = []
h_outils.append(MakePlane(t_point, MakeVectorDXDYDZ(1, 0, 0), g_trim))
h_outils.append(MakePlane(t_point, MakeVectorDXDYDZ(0, 1, 0), g_trim))

h_piece = MakePartition([t_piece], h_outils, [], [], ShapeType["SOLID"])

# Reparer
# -------

piece = RemoveExtraEdges(h_piece)

# Ajouter la piece dans l'etude
# -----------------------------

piece_id = addToStudy(piece, "ex18_dome2")

# Maillage
# ========

smesh.SetCurrentStudy(salome.myStudy)

# Maillage hexahedrique
# ---------------------

hexa = smesh.Mesh(piece, "ex18_dome2:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(2)

hexa.Quadrangle()

hexa.Hexahedron()

# Calcul du maillage
# ------------------

hexa.Compute()
