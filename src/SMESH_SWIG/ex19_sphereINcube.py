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

# ==================================
#
from geompy import *

import smesh
import geompy

# Geometrie
# =========

# Mailler en hexahedre une sphere dans un cube.

# Donnees
# -------

sphere_rayon = 100

cube_cote = 200

plan_trim = 1000

# Sphere
# ------

sphere_centre = MakeVertex(0, 0, 0)

sphere_pleine = MakeSpherePntR(sphere_centre, sphere_rayon)

# Cube interieur
# --------------

boite_cote = sphere_rayon / 2

boite = MakeBox(-boite_cote, -boite_cote, -boite_cote,  +boite_cote, +boite_cote, +boite_cote)

blocs = [boite]

# Decoupage sphere
# ----------------

sphere_troue = MakeCut(sphere_pleine, boite)

#sphere_outils = []
#sphere_outils.append(MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 0,  1), plan_trim))
#sphere_outils.append(MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 0, -1), plan_trim))
#sphere_outils.append(MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 1,  0), plan_trim))
#sphere_outils.append(MakePlane(sphere_centre, MakeVectorDXDYDZ(-1, 1,  0), plan_trim))

f1 = MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 0,  1), plan_trim)
f2 = MakePlane(sphere_centre, MakeVectorDXDYDZ(-1, 1,  0), plan_trim)
f3 = MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 1,  0), plan_trim)
f4 = MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 0, -1), plan_trim)


#sphere_decoupee = MakePartition(solids, sphere_outils, [], [], ShapeType["SOLID"])

sphere_decoupee = MakePartition([sphere_troue],    [f1], [], [], ShapeType["SOLID"])
sphere_decoupee = MakePartition([sphere_decoupee], [f2], [], [], ShapeType["SOLID"])
sphere_decoupee = MakePartition([sphere_decoupee], [f3], [], [], ShapeType["SOLID"])
sphere_decoupee = MakePartition([sphere_decoupee], [f4], [], [], ShapeType["SOLID"])

sphere_partie = geompy.MakeCompound([sphere_decoupee])

sphere_partie   = GetBlockNearPoint(sphere_decoupee, MakeVertex(-sphere_rayon, 0, 0))
sphere_bloc     = RemoveExtraEdges(sphere_partie)

blocs.append(sphere_bloc)

pi2 = 3.141592653/2

sphere_dir1 = MakeVectorDXDYDZ(0, 1,  0)
sphere_dir2 = MakeVectorDXDYDZ(0, 0,  1)

blocs.append(MakeRotation(sphere_bloc, sphere_dir1, +pi2))
blocs.append(MakeRotation(sphere_bloc, sphere_dir1, -pi2))

blocs.append(MakeRotation(sphere_bloc, sphere_dir2, +pi2))
blocs.append(MakeRotation(sphere_bloc, sphere_dir2, -pi2))

blocs.append(MakeMirrorByPoint(sphere_bloc, sphere_centre))

# Cube exterieur
# --------------

cube_plein   = MakeBox(-cube_cote, -cube_cote, -cube_cote,  +cube_cote, +cube_cote, +cube_cote)
cube_trou    = MakeCut(cube_plein, sphere_pleine)
#cube_decoupe = MakePartition([cube_trou], sphere_outils, [], [], ShapeType["SOLID"])

cube_decoupe = MakePartition([cube_trou],    [f1], [], [], ShapeType["SOLID"])
cube_decoupe = MakePartition([cube_decoupe], [f2], [], [], ShapeType["SOLID"])
cube_decoupe = MakePartition([cube_decoupe], [f3], [], [], ShapeType["SOLID"])
cube_decoupe = MakePartition([cube_decoupe], [f4], [], [], ShapeType["SOLID"])

cube_decoupe = geompy.MakeCompound([cube_decoupe])


cube_partie  = GetBlockNearPoint(cube_decoupe, MakeVertex(-cube_cote, 0, 0))
cube_bloc    = RemoveExtraEdges(cube_partie)

blocs.append(cube_bloc)

blocs.append(MakeRotation(cube_bloc, sphere_dir1, +pi2))
blocs.append(MakeRotation(cube_bloc, sphere_dir1, -pi2))

blocs.append(MakeRotation(cube_bloc, sphere_dir2, +pi2))
blocs.append(MakeRotation(cube_bloc, sphere_dir2, -pi2))

blocs.append(MakeMirrorByPoint(cube_bloc, sphere_centre))

# Piece
# -----

piece_cpd = MakeCompound(blocs)
piece = MakeGlueFaces(piece_cpd, 1.e-3)

piece_id = addToStudy(piece, "ex19_sphereINcube")

# Groupe geometrique
# ==================

# Definition du groupe
# --------------------

groupe = CreateGroup(piece, ShapeType["SOLID"])

groupe_nom = "ex19_sphereINcube_interieur"
addToStudy(groupe, groupe_nom)
groupe.SetName(groupe_nom)

# Contenu du groupe
# -----------------

groupe_sphere = GetShapesOnSphere(piece, ShapeType["SOLID"], sphere_centre, sphere_rayon, GEOM.ST_ONIN)

UnionList(groupe, groupe_sphere)

# Meshing
# =======

smesh.SetCurrentStudy(salome.myStudy)

# Create a hexahedral mesh
# ------------------------

hexa = smesh.Mesh(piece, "ex19_sphereINcube:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(10)

hexa.Quadrangle()

hexa.Hexahedron()

# Mesh calculus
# -------------

hexa.Compute()

# Le groupe de mailles
# --------------------

hexa_groupe = hexa.Group(groupe)
