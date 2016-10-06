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

# Geometrie
# =========

# Creer un cylindre avec un trou cylindrique excentre, decoupage en hexahedre et mailler.

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

g_trim = 15000

# Construire le cylindre
# ----------------------

c_point    = geompy.MakeVertex(g_ox, g_oy, g_oz-g_cyl_demiHauteur)
c_dir      = geompy.MakeVectorDXDYDZ(0, 0, 1)
c_hauteur  = 2*g_cyl_demiHauteur
c_cylindre = geompy.MakeCylinder(c_point, c_dir, g_cyl_rayon, c_hauteur)

# Trouer le cylindre par un minuscule cylindre excentre
# -----------------------------------------------------

t_hauteur = g_cyl_demiHauteur
t_point   = geompy.MakeVertex(g_ox-g_trou_centre, g_oy, g_oz-t_hauteur)
t_trou    = geompy.MakeCylinder(t_point, c_dir, g_trou_rayon, 2*t_hauteur)

t_piece   = geompy.MakeCut(c_cylindre, t_trou)

# Geometrie hexahedrique
# ======================

# Decouper
# --------

h_outils = []
h_outils.append(geompy.MakePlane(t_point, geompy.MakeVectorDXDYDZ(1, 0, 0), g_trim))
h_outils.append(geompy.MakePlane(t_point, geompy.MakeVectorDXDYDZ(0, 1, 0), g_trim))

h_piece = geompy.MakePartition([t_piece], h_outils, [], [], geompy.ShapeType["SOLID"])

# Decouper pour les conditions locales
# ------------------------------------

l_outils = []
l_i = 1
l_n = 12
l_hauteur = c_hauteur/l_n

while l_i<l_n:
    l_outils.append(geompy.MakePlane(geompy.MakeVertex(g_ox, g_oy, g_oz-g_cyl_demiHauteur+l_i*l_hauteur), c_dir, g_trim))
    l_i = l_i+1

piece = geompy.MakePartition([h_piece], l_outils, [], [], geompy.ShapeType["SOLID"])

# Ajouter la piece dans l'etude
# -----------------------------

piece_id = geompy.addToStudy(piece, "ex14_cyl1holed")

# Maillage
# ========

# Creer un maillage hexahedrique
# ------------------------------

hexa = smesh.Mesh(piece, "ex14_cyl1holed:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(4)

hexa.Quadrangle()

hexa.Hexahedron()

# Poser les hypotheses locales
# ----------------------------

m_i = 0
m_n = 12
m_h = c_hauteur/m_n
m_d = [4, 6, 8, 10, 10, 9, 8, 7, 6, 5, 4, 3]

m_x = g_ox+g_cyl_rayon
m_y = g_oy
m_z = g_oz-g_cyl_demiHauteur+m_h/2

while m_i<m_n:
    m_p = geompy.MakeVertex(m_x, m_y, m_z + m_i*m_h)
    m_e = geompy.GetEdgeNearPoint(piece, m_p)
    m_a = hexa.Segment(m_e)
    m_a.NumberOfSegments(m_d[m_i])
    m_a.Propagation()
    m_i = m_i + 1

# Calculer le maillage
# --------------------

hexa.Compute()

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
