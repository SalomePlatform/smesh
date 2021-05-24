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
"""problème de fissure plane coupant 2 faces (angle)"""

import os

from .cubeAngle import cubeAngle

class cubeAngle_2(cubeAngle):
  """problème de fissure plane coupant 2 faces (angle), débouches normaux, objet plan

  detection d'un probleme de tolerance sur les edges de jonction pipe et face fissure externe
  """

  nomProbleme = "cubeAngle_2"

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """paramètres de la fissure pour méthode construitFissureGenerale

    lgInfluence : distance autour de la shape de fissure a remailler (A ajuster selon le maillage)
    rayonPipe   : le rayon du pile maillé en hexa autour du fond de fissure
    """
    self.shapeFissureParams = dict(lgInfluence = 20,
                                   rayonPipe   = 5)

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep           = os.curdir,
                                      nomFicSain       = self.nomProbleme,
                                      nomFicFissure    = self.nomProbleme + "_fissure",
                                      nbsegRad         = 5,
                                      nbsegCercle      = 32,
                                      areteFaceFissure = 5)

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 2350, \
                                          Entity_Quad_Hexa = 5315, \
                                          Entity_Node = 38196, \
                                          Entity_Quad_Edge = 345, \
                                          Entity_Quad_Triangle = 1214, \
                                          Entity_Quad_Tetra = 7772, \
                                          Entity_Quad_Pyramid = 620, \
                                          Entity_Quad_Penta = 512 \
                                         )
