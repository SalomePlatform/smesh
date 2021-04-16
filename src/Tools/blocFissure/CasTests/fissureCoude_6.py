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
"""Fissure dans un coude - ASCOU08"""

import os

from blocFissure.gmu.fissureCoude  import fissureCoude

class fissureCoude_6(fissureCoude):
  """ASCOU08"""

  nomProbleme = "fissureCoude_6"
  geomParams = dict()
  meshParams = dict()
  shapeFissureParams = dict()
  maillageFissureParams = dict()
  referencesMaillageFissure = dict()

# ---------------------------------------------------------------------------
  def setParamGeometrieSaine(self):
    """
    Paramètres géométriques du tuyau coudé sain:
    angleCoude
    r_cintr
    l_tube_p1
    l_tube_p2
    epais
    de
    """
    self.geomParams = dict(angleCoude = 40,
                           r_cintr    = 854,
                           l_tube_p1  = 1700,
                           l_tube_p2  = 1700,
                           epais      = 62.5,
                           de         = 912.4)

  # ---------------------------------------------------------------------------
  def setParamMaillageSain(self):
    self.meshParams = dict(n_long_p1    = 15,
                           n_ep         = 3,
                           n_long_coude = 16,
                           n_circ_g     = 30,
                           n_circ_d     = 30,
                           n_long_p2    = 15)

# ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour le tuyau coude
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
    self.shapeFissureParams = dict(profondeur  = 10,
                                   rayonPipe   = 4,
                                   lenSegPipe  = 4,
                                   azimut      = 90,
                                   alpha       = 20,
                                   longueur    = 133,
                                   orientation = 0,
                                   lgInfluence = 30,
                                   elliptique  = False,
                                   externe     = True)

  # ---------------------------------------------------------------------------

  def setParamMaillageFissure(self):
    """
    Paramètres du maillage de la fissure pour le tuyau coudé
    Voir également setParamShapeFissure, paramètres rayonPipe et lenSegPipe.
    nbSegRad = nombre de couronnes
    nbSegCercle = nombre de secteurs
    areteFaceFissure = taille cible de l'arête des triangles en face de fissure.
    """
    self.maillageFissureParams = dict(nomRep        = os.curdir,
                                      nomFicSain    = self.nomProbleme,
                                      nomFicFissure = self.nomProbleme + "_fissure",
                                      nbsegRad      = 5,
                                      nbsegCercle   = 12,
                                      areteFaceFissure = 5)

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 7392, \
                                          Entity_Quad_Hexa = 9888, \
                                          Entity_Node = 62571, \
                                          Entity_Quad_Edge = 833, \
                                          Entity_Quad_Triangle = 1048, \
                                          Entity_Quad_Tetra = 7723, \
                                          Entity_Quad_Pyramid = 452, \
                                          Entity_Quad_Penta = 408 \
                                         )
