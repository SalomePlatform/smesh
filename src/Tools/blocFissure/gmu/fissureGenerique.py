# -*- coding: utf-8 -*-
# Copyright (C) 2014-2019  CEA/DEN, EDF R&D
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

import logging

from blocFissure import gmu
from blocFissure.gmu.initEtude import initEtude
from blocFissure.gmu.getStatsMaillageFissure import getStatsMaillageFissure

class fissureGenerique():
  """
  classe générique problème fissure:
  génération géométrie et maillage sain
  définition et positionnement d'une fissure
  génération d'un bloc défaut inséré dans le maillage sain
  """

  nomProbleme = "generique"

  def __init__(self, numeroCas):
    initEtude()
    self.numeroCas = numeroCas
    self.nomCas = self.nomProbleme +"_%d"%(self.numeroCas)
    self.fissureLongue = False

  def setParamGeometrieSaine(self):
    self.geomParams = {}

  def genereGeometrieSaine(self, geomParams):
    geometriesSaines = [None]
    return geometriesSaines

  def setParamMaillageSain(self):
    self.meshParams = {}

  def genereMaillageSain(self, geometriesSaines, meshParams):
    maillagesSains = [None]
    return maillagesSains

  def setParamShapeFissure(self):
    self.shapeFissureParams = {}

  def genereShapeFissure(self, geometriesSaines, geomParams, shapeFissureParams):
    shapesFissure = [None]
    return shapesFissure

  def setParamMaillageFissure(self):
    self.maillageFissureParams = {}

  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, maillageFissureParams):
    elementsDefaut = [None]
    return elementsDefaut

  def genereMaillageFissure(self, geometriesSaines, maillagesSains, shapesFissure,
                            maillageFissureParams, elementsDefaut, step):
    maillageFissure = None
    return maillageFissure

  def setReferencesMaillageFissure(self):
    referencesMaillageFissure = {}
    return referencesMaillageFissure

# ---------------------------------------------------------------------------

  def executeProbleme(self, step=-1):
    logging.info(" --- executeProbleme %s", self.nomCas)
    if step == 0:
      return

    self.setParamGeometrieSaine()
    geometriesSaines = self.genereGeometrieSaine(self.geomParams)
    if step == 1:
      return

    self.setParamMaillageSain()
    maillagesSains = self.genereMaillageSain(geometriesSaines, self.meshParams)
    if step == 2:
      return

    self.setParamShapeFissure()
    shapesFissure = self.genereShapeFissure(geometriesSaines, self.geomParams, self.shapeFissureParams)
    if step == 3:
      return

    self.setParamMaillageFissure()
    elementsDefaut = self.genereZoneDefaut(geometriesSaines, maillagesSains, shapesFissure, self.shapeFissureParams, self.maillageFissureParams)
    if step == 4:
      return

    maillageFissure = self.genereMaillageFissure(geometriesSaines, maillagesSains,
                                                 shapesFissure, self.shapeFissureParams,
                                                 self.maillageFissureParams, elementsDefaut, step)

    self.setReferencesMaillageFissure()
    mesures = getStatsMaillageFissure(maillageFissure, self.referencesMaillageFissure, self.maillageFissureParams)






