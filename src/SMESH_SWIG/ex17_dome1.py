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

# Creer un cylindre surplombe d'une demi-sphere le tout troue par un petit cylindre.
# Decouper en hexahedre et mailler.

# Donnees
# -------

cylindre_rayon   = 100
cylindre_hauteur = 400

trou_rayon = 20
trou_z     = cylindre_rayon/2

plan_trim = 2000

# Cylindre
# --------

cylindre_base = geompy.MakeVertex(0, 0, 0)
cylindre_dir  = geompy.MakeVectorDXDYDZ(1, 0, 0)
cylindre      = geompy.MakeCylinder(cylindre_base, cylindre_dir, cylindre_rayon, cylindre_hauteur)

# Dome
# ----

dome_sphere = geompy.MakeSpherePntR(cylindre_base, cylindre_rayon)
dome        = geompy.MakeFuse(dome_sphere, cylindre)

# Cheminee
# --------

cheminee_base = geompy.MakeVertex(-cylindre_hauteur/2, 0, trou_z)
cheminee_trou = geompy.MakeCylinder(cheminee_base, cylindre_dir, trou_rayon, 2*cylindre_hauteur)
cheminee      = geompy.MakeCut(dome, cheminee_trou)

# Decoupage et reparation
# -----------------------

blocs_plan1 = geompy.MakePlane(cheminee_base, geompy.MakeVectorDXDYDZ(0, 1, 0), plan_trim)
blocs_plan2 = geompy.MakePlane(cheminee_base, geompy.MakeVectorDXDYDZ(0, 0, 1), plan_trim)

blocs_part = geompy.MakePartition([cheminee], [blocs_plan1, blocs_plan2], [], [], geompy.ShapeType["SOLID"])

piece = geompy.RemoveExtraEdges(blocs_part)

# Etude
# -----

piece_id = geompy.addToStudy(piece, "ex17_dome1")

# Maillage
# ========

# Maillage hexahedrique
# ---------------------

hexa = smesh.Mesh(piece, "ex17_dome1:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(20)

hexa.Quadrangle()

hexa.Hexahedron()

# Calcul du maillage
# ------------------

hexa.Compute()
# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
