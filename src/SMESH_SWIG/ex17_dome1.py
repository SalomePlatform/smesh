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

cylindre_base = MakeVertex(0, 0, 0)
cylindre_dir  = MakeVectorDXDYDZ(1, 0, 0)
cylindre      = MakeCylinder(cylindre_base, cylindre_dir, cylindre_rayon, cylindre_hauteur)

# Dome
# ----

dome_sphere = MakeSpherePntR(cylindre_base, cylindre_rayon)
dome        = MakeFuse(dome_sphere, cylindre)

# Cheminee
# --------

cheminee_base = MakeVertex(-cylindre_hauteur/2, 0, trou_z)
cheminee_trou = MakeCylinder(cheminee_base, cylindre_dir, trou_rayon, 2*cylindre_hauteur)
cheminee      = MakeCut(dome, cheminee_trou)

# Decoupage et reparation
# -----------------------

blocs_plan1 = MakePlane(cheminee_base, MakeVectorDXDYDZ(0, 1, 0), plan_trim)
blocs_plan2 = MakePlane(cheminee_base, MakeVectorDXDYDZ(0, 0, 1), plan_trim)

blocs_part = MakePartition([cheminee], [blocs_plan1, blocs_plan2], [], [], ShapeType["SOLID"])

piece = RemoveExtraEdges(blocs_part)

# Etude
# -----

piece_id = addToStudy(piece, "ex17_dome1")

# Maillage
# ========

smesh.SetCurrentStudy(salome.myStudy)

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
