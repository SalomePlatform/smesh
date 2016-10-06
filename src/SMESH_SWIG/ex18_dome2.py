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

c_point    = geompy.MakeVertex(g_ox, g_oy, g_oz-g_cyl_demiHauteur)
c_dir      = geompy.MakeVectorDXDYDZ(0, 0, 1)
c_hauteur  = 2*g_cyl_demiHauteur

c_cylindre = geompy.MakeCylinder(c_point, c_dir, g_cyl_rayon, c_hauteur)

# Sphere
# ------

s_hauteur = math.sqrt(g_sphere_rayon*g_sphere_rayon - g_cyl_rayon*g_cyl_rayon) - g_cyl_demiHauteur

s_sphere  = geompy.MakeSphere(g_ox, g_oy, g_oz-s_hauteur, g_sphere_rayon)

# Calottes
# --------

c_outils = []
c_outils.append(geompy.MakePlane(geompy.MakeVertex(g_ox, g_oy, g_oz+g_cyl_demiHauteur), geompy.MakeVectorDXDYDZ(0, 0, 1), g_trim))

c_cpd = geompy.MakePartition([s_sphere], c_outils, [], [], geompy.ShapeType["SOLID"])
c_calotte_haut, c_reste = geompy.SubShapeAllSorted(c_cpd, geompy.ShapeType["SOLID"])

c_plan = geompy.MakePlane(geompy.MakeVertex(g_ox, g_oy, g_oz), geompy.MakeVectorDXDYDZ(0, 0, 1), g_trim)
c_calotte_bas = geompy.MakeMirrorByPlane(c_calotte_haut, c_plan)

# Fusionner
# ---------

f_piece1 = geompy.MakeFuse(c_cylindre, c_calotte_haut)
f_piece  = geompy.MakeFuse(f_piece1, c_calotte_bas)

# Trouer
# ------

t_hauteur = g_sphere_rayon
t_point   = geompy.MakeVertex(g_ox-g_trou_centre, g_oy, g_oz-t_hauteur)
t_trou    = geompy.MakeCylinder(t_point, c_dir, g_trou_rayon, 2*t_hauteur)

t_piece   = geompy.MakeCut(f_piece, t_trou)

# Decouper
# --------

h_outils = []
h_outils.append(geompy.MakePlane(t_point, geompy.MakeVectorDXDYDZ(1, 0, 0), g_trim))
h_outils.append(geompy.MakePlane(t_point, geompy.MakeVectorDXDYDZ(0, 1, 0), g_trim))

h_piece = geompy.MakePartition([t_piece], h_outils, [], [], geompy.ShapeType["SOLID"])

# Reparer
# -------

piece = geompy.RemoveExtraEdges(h_piece)

# Ajouter la piece dans l'etude
# -----------------------------

piece_id = geompy.addToStudy(piece, "ex18_dome2")

# Maillage
# ========

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

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
