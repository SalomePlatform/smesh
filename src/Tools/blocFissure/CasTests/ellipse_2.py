# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
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

"""problème de fissure non plane, débouchante non normale"""

import os
import logging

from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy, smesh
from blocFissure.gmu.fissureGenerique import fissureGenerique
from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from blocFissure.gmu.creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from blocFissure.gmu.construitFissureGenerale import construitFissureGenerale

import GEOM
import SALOMEDS
import SMESH

from .ellipse_1 import ellipse_1

class ellipse_2(ellipse_1):
  """problème de fissure non plane, débouchante non normale"""

  nomProbleme = "ellipse2"

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams):
    """Importe la géométrie de la fissure"""
    texte = "genereShapeFissure pour '{}'".format(self.nomCas)
    logging.info(texte)

    lgInfluence = shapeFissureParams['lgInfluence']

    shellFiss = geompy.ImportBREP(os.path.join(gmu.pathBloc, "materielCasTests", "ellipse1_pb.brep"))
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(fondFiss, [3])
    geompy.addToStudy( shellFiss, 'shellFiss' )
    geompy.addToStudyInFather( shellFiss, fondFiss, 'fondFiss' )

    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, 5 ,25)

    centre = None

    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 1748, \
                                          Entity_Quad_Hexa = 3795, \
                                          Entity_Node = 22219, \
                                          Entity_Quad_Edge = 258, \
                                          Entity_Quad_Triangle = 434, \
                                          Entity_Quad_Tetra = 2574, \
                                          Entity_Quad_Pyramid = 199, \
                                          Entity_Quad_Penta = 120 \
                                         )
