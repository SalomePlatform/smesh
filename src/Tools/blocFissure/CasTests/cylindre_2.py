# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
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

"""problème de fissure plane sur cylindre hexa, fond de fissure complexe : polyline"""

import os
import logging

from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy

from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut

from .cylindre import cylindre

class cylindre_2(cylindre):
  """problème de fissure plane sur cylindre hexa, fond de fissure complexe : polyline"""

  nomProbleme = "cylindre_2"

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams, \
                                mailleur="MeshGems"):
    logging.info("genereShapeFissure %s", self.nomCas)

    lgInfluence = shapeFissureParams['lgInfluence']

    shellFiss = geompy.ImportBREP(os.path.join(gmu.pathBloc, "materielCasTests", "FissInCylindre.brep"))
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(fondFiss, [6])
    geompy.addToStudy( shellFiss, 'shellFiss' )
    geompy.addToStudyInFather( shellFiss, fondFiss, 'fondFiss' )

    mailleur = self.mailleur2d3d()
    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, 5 ,15, mailleur, self.numeroCas)

    centre = None
    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 7028, \
                                          Entity_Quad_Hexa = 18366, \
                                          Entity_Node = 105035, \
                                          Entity_Quad_Edge = 735, \
                                          Entity_Quad_Triangle = 1056, \
                                          Entity_Quad_Tetra = 16305, \
                                          Entity_Quad_Pyramid = 1158, \
                                          Entity_Quad_Penta = 384 \
                                          )
