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

"""problème de fissure plane dans un tube ; passage par fichier xao"""

import os
import logging

from blocFissure import gmu
from blocFissure.gmu import initLog
from blocFissure.gmu.geomsmesh import geompy, smesh
from blocFissure.gmu.geomsmesh import geomPublish
from blocFissure.gmu.geomsmesh import geomPublishInFather

from blocFissure.gmu.fissureGenerique import fissureGenerique
from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from blocFissure.gmu.creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from blocFissure.gmu.construitFissureGenerale import construitFissureGenerale
from blocFissure.gmu.putName import putName

class tube(fissureGenerique):
  """problème de fissure plane dans un tube"""

  nomProbleme = "tube"
  shapeFissureParams = dict()
  maillageFissureParams = dict()
  referencesMaillageFissure = dict()

  # ---------------------------------------------------------------------------
  def genereMaillageSain(self, geometriesSaines, meshParams):
    texte = "genereMaillageSain pour '{}'".format(self.nomCas)
    logging.info(texte)

    ([objetSain], _) = smesh.CreateMeshesFromMED(os.path.join(gmu.pathBloc, "materielCasTests", "Tube.med"))
    putName(objetSain.GetMesh(), self.nomProbleme, i_pref=self.numeroCas)

    return [objetSain, True] # True : maillage hexa

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour méthode construitFissureGenerale
    lgInfluence : distance autour de la shape de fissure a remailler (A ajuster selon le maillage)
    rayonPipe   : le rayon du pile maillé en hexa autour du fond de fissure
    """
    texte = "genereMaillageSain pour '{}'".format(self.nomCas)
    logging.info(texte)
    self.shapeFissureParams = dict(lgInfluence = 0.4,
                                   rayonPipe   = 0.05)

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams, \
                                mailleur="MeshGems"):
    """Importe la géométrie de la fissure"""
    texte = "genereShapeFissure pour '{}'".format(self.nomCas)
    logging.info(texte)

    lgInfluence = shapeFissureParams['lgInfluence']

    (_, shellFiss, _, l_groups, _) = geompy.ImportXAO(os.path.join(gmu.pathBloc, "materielCasTests", "TubeFiss.xao"))
    l_aux = list()
    for group in l_groups:
      if ( group.GetName() in ("Group_1","Group_2","Group_3") ):
        l_aux.append(group)
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
    geompy.UnionList(fondFiss, l_aux )
    geomPublish(initLog.always, shellFiss, "shellFiss", self.numeroCas)
    geomPublishInFather(initLog.always, shellFiss, fondFiss, "fondFiss", self.numeroCas)

    mailleur = self.mailleur2d3d()
    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, 0.025, 0.1, mailleur, self.numeroCas)

    centre = None

    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    texte = "setParamMaillageFissure pour '{}'".format(self.nomCas)
    logging.info(texte)
    self.maillageFissureParams = dict(nomRep           = os.curdir,
                                      nomFicSain       = self.nomProbleme,
                                      nomFicFissure    = self.nomProbleme + "_fissure",
                                      nbsegRad         = 5,
                                      nbsegCercle      = 8,
                                      areteFaceFissure = 0.5)

  # ---------------------------------------------------------------------------
  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams):
    elementsDefaut = creeZoneDefautDansObjetSain(geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams, \
                                                 self.numeroCas)
    return elementsDefaut

  # ---------------------------------------------------------------------------
  def genereMaillageFissure(self, geometriesSaines, maillagesSains, \
                                  shapesFissure, shapeFissureParams, \
                                  maillageFissureParams, elementsDefaut, step, \
                                  mailleur="MeshGems"):

    texte = "genereMaillageFissure pour '{}'".format(self.nomCas)
    logging.info(texte)

    mailleur = self.mailleur2d3d()
    maillageFissure = construitFissureGenerale(shapesFissure, shapeFissureParams, \
                                               maillageFissureParams, elementsDefaut, \
                                               mailleur, self.numeroCas)
    return maillageFissure

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 1630, \
                                          Entity_Quad_Hexa = 3523, \
                                          Entity_Node = 18874, \
                                          Entity_Quad_Edge = 261, \
                                          Entity_Quad_Triangle = 190, \
                                          Entity_Quad_Tetra = 1322, \
                                          Entity_Quad_Pyramid = 172, \
                                          Entity_Quad_Penta = 64 \
                                         )
