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
"""Fissure dans un coude - ASCOU09A"""

import os

from .fissure_Coude  import fissure_Coude

class fissure_Coude_4(fissure_Coude):
  """probleme de fissure du Coude : ASCOU09A

  adaptation maillage
  """

  nomProbleme = "fissure_Coude_4"
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
                           r_cintr    = 654,
                           l_tube_p1  = 1700,
                           l_tube_p2  = 1700,
                           epais      = 62.5,
                           de         = 912.4)

  # ---------------------------------------------------------------------------
  def setParamMaillageSain(self):
    self.meshParams = dict(n_long_p1    = 16,
                           n_ep         = 5,
                           n_long_coude = 30,
                           n_circ_g     = 50,
                           n_circ_d     = 20,
                           n_long_p2    = 12)

# ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure
    profondeur  : 0 < profondeur <= épaisseur
    azimut      : entre 0 et 360°
    alpha       : 0 < alpha < angleCoude
    longueur    : <=2*profondeur ==> ellipse, >2*profondeur = fissure longue
    orientation : 0° : longitudinale, 90° : circonférentielle, autre : uniquement fissures elliptiques
    externe     : True : fissure face externe, False : fissure face interne
    """
    self.shapeFissureParams = dict(nomRep        = os.curdir,
                                   nomFicSain    = self.nomCas,
                                   nomFicFissure = self.nomProbleme + "_fissure",
                                   profondeur  = 10,
                                   azimut      = 90,
                                   alpha       = 20,
                                   longueur    = 240,
                                   orientation = 90,
                                   lgInfluence = 30,
                                   elliptique  = False,
                                   convexe     = True,
                                   externe     = True)

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 11852, \
                                          Entity_Quad_Hexa       = 22208, \
                                          Entity_Node            = 133418, \
                                          Entity_Quad_Edge       = 1192, \
                                          Entity_Quad_Triangle   = 1562, \
                                          Entity_Quad_Tetra      = 18759, \
                                          Entity_Quad_Pyramid    = 948, \
                                          Entity_Quad_Penta      = 732, \
                                         )
