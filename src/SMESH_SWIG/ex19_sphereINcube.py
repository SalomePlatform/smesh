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

# ==================================
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

# Mailler en hexahedre une sphere dans un cube.

# Donnees
# -------

sphere_rayon = 100

cube_cote = 200

plan_trim = 1000

# Sphere
# ------

sphere_centre = geompy.MakeVertex(0, 0, 0)

sphere_pleine = geompy.MakeSpherePntR(sphere_centre, sphere_rayon)

# Cube interieur
# --------------

boite_cote = sphere_rayon / 2

boite = geompy.MakeBox(-boite_cote, -boite_cote, -boite_cote,  +boite_cote, +boite_cote, +boite_cote)

blocs = [boite]

# Decoupage sphere
# ----------------

sphere_troue = geompy.MakeCut(sphere_pleine, boite)

#sphere_outils = []
#sphere_outils.append(MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 0,  1), plan_trim))
#sphere_outils.append(MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 0, -1), plan_trim))
#sphere_outils.append(MakePlane(sphere_centre, MakeVectorDXDYDZ( 1, 1,  0), plan_trim))
#sphere_outils.append(MakePlane(sphere_centre, MakeVectorDXDYDZ(-1, 1,  0), plan_trim))

f1 = geompy.MakePlane(sphere_centre, geompy.MakeVectorDXDYDZ( 1, 0,  1), plan_trim)
f2 = geompy.MakePlane(sphere_centre, geompy.MakeVectorDXDYDZ(-1, 1,  0), plan_trim)
f3 = geompy.MakePlane(sphere_centre, geompy.MakeVectorDXDYDZ( 1, 1,  0), plan_trim)
f4 = geompy.MakePlane(sphere_centre, geompy.MakeVectorDXDYDZ( 1, 0, -1), plan_trim)


#sphere_decoupee = MakePartition(solids, sphere_outils, [], [], ShapeType["SOLID"])

sphere_decoupee = geompy.MakePartition([sphere_troue],    [f1], [], [], geompy.ShapeType["SOLID"])
sphere_decoupee = geompy.MakePartition([sphere_decoupee], [f2], [], [], geompy.ShapeType["SOLID"])
sphere_decoupee = geompy.MakePartition([sphere_decoupee], [f3], [], [], geompy.ShapeType["SOLID"])
sphere_decoupee = geompy.MakePartition([sphere_decoupee], [f4], [], [], geompy.ShapeType["SOLID"])

sphere_partie = geompy.MakeCompound([sphere_decoupee])

sphere_partie   = geompy.GetBlockNearPoint(sphere_decoupee, geompy.MakeVertex(-sphere_rayon, 0, 0))
sphere_bloc     = geompy.RemoveExtraEdges(sphere_partie)

blocs.append(sphere_bloc)

pi2 = 3.141592653/2

sphere_dir1 = geompy.MakeVectorDXDYDZ(0, 1,  0)
sphere_dir2 = geompy.MakeVectorDXDYDZ(0, 0,  1)

blocs.append(geompy.MakeRotation(sphere_bloc, sphere_dir1, +pi2))
blocs.append(geompy.MakeRotation(sphere_bloc, sphere_dir1, -pi2))

blocs.append(geompy.MakeRotation(sphere_bloc, sphere_dir2, +pi2))
blocs.append(geompy.MakeRotation(sphere_bloc, sphere_dir2, -pi2))

blocs.append(geompy.MakeMirrorByPoint(sphere_bloc, sphere_centre))

# Cube exterieur
# --------------

cube_plein   = geompy.MakeBox(-cube_cote, -cube_cote, -cube_cote,  +cube_cote, +cube_cote, +cube_cote)
cube_trou    = geompy.MakeCut(cube_plein, sphere_pleine)
#cube_decoupe = MakePartition([cube_trou], sphere_outils, [], [], ShapeType["SOLID"])

cube_decoupe = geompy.MakePartition([cube_trou],    [f1], [], [], geompy.ShapeType["SOLID"])
cube_decoupe = geompy.MakePartition([cube_decoupe], [f2], [], [], geompy.ShapeType["SOLID"])
cube_decoupe = geompy.MakePartition([cube_decoupe], [f3], [], [], geompy.ShapeType["SOLID"])
cube_decoupe = geompy.MakePartition([cube_decoupe], [f4], [], [], geompy.ShapeType["SOLID"])

cube_decoupe = geompy.MakeCompound([cube_decoupe])


cube_partie  = geompy.GetBlockNearPoint(cube_decoupe, geompy.MakeVertex(-cube_cote, 0, 0))
cube_bloc    = geompy.RemoveExtraEdges(cube_partie)

blocs.append(cube_bloc)

blocs.append(geompy.MakeRotation(cube_bloc, sphere_dir1, +pi2))
blocs.append(geompy.MakeRotation(cube_bloc, sphere_dir1, -pi2))

blocs.append(geompy.MakeRotation(cube_bloc, sphere_dir2, +pi2))
blocs.append(geompy.MakeRotation(cube_bloc, sphere_dir2, -pi2))

blocs.append(geompy.MakeMirrorByPoint(cube_bloc, sphere_centre))

# Piece
# -----

piece_cpd = geompy.MakeCompound(blocs)
piece = geompy.MakeGlueFaces(piece_cpd, 1.e-3)

piece_id = geompy.addToStudy(piece, "ex19_sphereINcube")

# Groupe geometrique
# ==================

# Definition du groupe
# --------------------

groupe = geompy.CreateGroup(piece, geompy.ShapeType["SOLID"])

groupe_nom = "ex19_sphereINcube_interieur"
geompy.addToStudy(groupe, groupe_nom)
groupe.SetName(groupe_nom)

# Contenu du groupe
# -----------------

groupe_sphere = geompy.GetShapesOnSphere(piece, geompy.ShapeType["SOLID"], sphere_centre, sphere_rayon, GEOM.ST_ONIN)

geompy.UnionList(groupe, groupe_sphere)

# Meshing
# =======

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

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
