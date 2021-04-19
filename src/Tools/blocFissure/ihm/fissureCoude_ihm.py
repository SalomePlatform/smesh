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

import os

from blocFissure.gmu.fissureCoude  import fissureCoude

class fissureCoude_ihm(fissureCoude):
  """
  problème de fissure du Coude :
  pour version IHM graphique
  """

# ---------------------------------------------------------------------------
  def setDicoParams(self,dico):
    self.dico = dico

# ---------------------------------------------------------------------------
  def setParamGeometrieSaine(self):
    """Paramètres géométriques du tuyau coudé sain:

    angleCoude
    r_cintr
    l_tube_p1
    l_tube_p2
    epais
    de
    """
    self.geomParams = dict(angleCoude = self.dico['angle'], \
                           r_cintr    = self.dico['rCintr'], \
                           l_tube_p1  = self.dico['lTubeP1'], \
                           l_tube_p2  = self.dico['lTubeP2'], \
                           epais      = self.dico['epais'], \
                           de         = self.dico['dext'])

  # ---------------------------------------------------------------------------
  def setParamMaillageSain(self):
    self.meshParams = dict(n_long_p1    = self.dico['nbAxeTubeP1'], \
                           n_ep         = self.dico['nbEpaisseur'], \
                           n_long_coude = self.dico['nbAxeCoude'], \
                           n_circ_g     = self.dico['nbCirconf'], \
                           n_circ_d     = self.dico['nbCirconf'], \
                           n_long_p2    = self.dico['nbAxeTubeP2'])

# ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """paramètres de la fissure pour le tuyau coude

    profondeur  : 0 < profondeur <= épaisseur
    rayonPipe   : rayon du pipe correspondant au maillage rayonnant
    lenSegPipe  : longueur des mailles rayonnantes le long du fond de fissure (= rayonPipe par défaut)
    azimut      : entre 0 et 360°
    alpha       : 0 < alpha < angleCoude
    longueur    : <=2*profondeur ==> force une fissure elliptique (longueur/profondeur = grand axe/petit axe).
    orientation : 0° : longitudinale, 90° : circonférentielle, autre : uniquement fissures elliptiques
    lgInfluence : distance autour de la shape de fissure a remailler (si 0, pris égal à profondeur. A ajuster selon le maillage)
    elliptique  : True : fissure elliptique (longueur/profondeur = grand axe/petit axe); False : fissure longue (fond de fissure de profondeur constante, demi-cercles aux extrémites)
    pointIn_x   : optionnel coordonnées x d'un point dans le solide, pas trop loin du centre du fond de fissure (idem y,z)
    externe     : True : fissure face externe, False : fissure face interne
    """
    print("setParamShapeFissure", self.nomCas)
    self.shapeFissureParams = dict(profondeur  = self.dico['profondeur'], \
                                   rayonPipe   = self.dico['rayonTore'], \
                                   lenSegPipe  = self.dico['lenSegPipe'], \
                                   azimut      = self.dico['azimut'], \
                                   alpha       = self.dico['posiAngul'], \
                                   longueur    = self.dico['longueur'], \
                                   orientation = self.dico['orientation'], \
                                   lgInfluence = self.dico['influence'], \
                                   elliptique  = self.dico['cbForceEllipse'], \
                                   externe     = self.dico['rbFissExt'])

# ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    """Paramètres du maillage de la fissure pour le tuyau coudé

    Voir également setParamShapeFissure, paramètres rayonPipe et lenSegPipe.
    nbSegRad = nombre de couronnes
    nbSegCercle = nombre de secteurs
    areteFaceFissure = taille cible de l'arête des triangles en face de fissure.
    """
    self.maillageFissureParams = dict(nomRep        = os.curdir, \
                                      nomFicSain    = self.nomProbleme, \
                                      nomFicFissure = self.nomProbleme + '_fissure', \
                                      nbsegRad      = self.dico['nbCouronnes'], \
                                      nbsegCercle   = self.dico['nbSecteurs'], \
                                      areteFaceFissure = self.dico['aretesFaceFissure'])

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 0, \
                                          Entity_Quad_Hexa = 0, \
                                          Entity_Node = 0, \
                                          Entity_Quad_Edge = 0, \
                                          Entity_Quad_Triangle = 0, \
                                          Entity_Quad_Tetra = 0, \
                                          Entity_Quad_Pyramid = 0, \
                                          Entity_Quad_Penta = 0 \
                                          )

