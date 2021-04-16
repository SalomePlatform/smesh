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

"""problème de fissure plane sur cylindre, grand fond de fissure en arc de cercle"""

import os
import logging

from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy, smesh
from blocFissure.gmu.fissureGenerique import fissureGenerique
from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from blocFissure.gmu.creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from blocFissure.gmu.construitFissureGenerale import construitFissureGenerale
from blocFissure.gmu.putName import putName

class cylindre(fissureGenerique):
  """problème de fissure plane sur cylindre, grand fond de fissure en arc de cercle"""

  nomProbleme = "cylindre"
  shapeFissureParams = dict()
  maillageFissureParams = dict()
  referencesMaillageFissure = dict()

  # ---------------------------------------------------------------------------
  def genereMaillageSain(self, geometriesSaines, meshParams):
    """
    génère le maillage de l'objet sain, par chargement d'un fichier med
    ici, les paramètres de géométrie et de maillage ne sont pas utiles
    """
    logging.info("genereMaillageSain %s", self.nomCas)

    ([objetSain], _) = smesh.CreateMeshesFromMED(os.path.join(gmu.pathBloc, "materielCasTests", "CylindreSain.med"))
    putName(objetSain.GetMesh(), self.nomProbleme, i_pref=self.numeroCas)

    return [objetSain, True] # True : maillage hexa

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour méthode construitFissureGenerale
    lgInfluence : distance autour de la shape de fissure a remailler (A ajuster selon le maillage)
    rayonPipe   : le rayon du pile maillé en hexa autour du fond de fissure
    """
    logging.info("setParamShapeFissure %s", self.nomCas)
    self.shapeFissureParams = dict(lgInfluence = 20,
                                   rayonPipe   = 5)

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams, \
                                mailleur="MeshGems"):
    logging.info("genereShapeFissure %s", self.nomCas)

    lgInfluence = shapeFissureParams['lgInfluence']

    shellFiss = geompy.ImportBREP(os.path.join(gmu.pathBloc, "materielCasTests", "FissInCylindre2.brep"))
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(fondFiss, [6])
    geompy.addToStudy( shellFiss, 'shellFiss' )
    geompy.addToStudyInFather( shellFiss, fondFiss, 'fondFiss' )

    mailleur = self.mailleur2d3d()
    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, 5 ,15, mailleur, self.numeroCas)

    centre = None
    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep           = os.curdir,
                                      nomFicSain       = self.nomProbleme,
                                      nomFicFissure    = self.nomProbleme + "_fissure",
                                      nbsegRad         = 5,
                                      nbsegCercle      = 8,
                                      areteFaceFissure = 20)

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

    mailleur = self.mailleur2d3d()
    maillageFissure = construitFissureGenerale(shapesFissure, shapeFissureParams, \
                                               maillageFissureParams, elementsDefaut, \
                                               mailleur, self.numeroCas)
    return maillageFissure

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 6920, \
                                          Entity_Quad_Hexa = 18174, \
                                          Entity_Node = 104349, \
                                          Entity_Quad_Edge = 718, \
                                          Entity_Quad_Triangle = 952, \
                                          Entity_Quad_Tetra = 16691, \
                                          Entity_Quad_Pyramid = 1110, \
                                          Entity_Quad_Penta = 336 \
                                         )
