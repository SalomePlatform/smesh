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
"""fissureGenerique"""

import logging

from blocFissure import gmu
from .initEtude import initEtude
from .getStatsMaillageFissure import getStatsMaillageFissure

class fissureGenerique(object):
  """classe générique problème fissure:

  génération géométrie et maillage sain
  définition et positionnement d'une fissure
  génération d'un bloc défaut inséré dans le maillage sain
  """

  nomProbleme = "fissureGenerique"
  maillageFissure = None
  geomParams = dict()
  meshParams = dict()
  shapeFissureParams = dict()
  maillageFissureParams = dict()

  def __init__(self, numeroCas):
    initEtude()
    self.numeroCas = numeroCas
    self.nomCas = self.nomProbleme +"_%d"%(self.numeroCas)
    self.fissureLongue = False
    self.referencesMaillageFissure = None

  def setParamGeometrieSaine(self):
    """setParamGeometrieSaine"""
    self.geomParams = dict()

  def genereGeometrieSaine(self, geomParams):
    """genereGeometrieSaine"""
    geometriesSaines = [None]
    return geometriesSaines

  def setParamMaillageSain(self):
    """setParamMaillageSain"""
    self.meshParams = dict()

  def genereMaillageSain(self, geometriesSaines, meshParams):
    """genereMaillageSain"""
    maillagesSains = [None]
    return maillagesSains

  def setParamShapeFissure(self):
    """setParamShapeFissure"""
    self.shapeFissureParams = dict()

  def genereShapeFissure(self, geometriesSaines, geomParams, shapeFissureParams, mailleur="MeshGems"):
    """genereShapeFissure"""
    shapesFissure = [None]
    return shapesFissure

  def setParamMaillageFissure(self):
    """setParamMaillageFissure"""
    self.maillageFissureParams = dict()

  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams):
    """genereZoneDefaut"""
    elementsDefaut = [None]
    return elementsDefaut

  def genereMaillageFissure(self, geometriesSaines, maillagesSains, \
                                  shapesFissure, shapeFissureParams, \
                                  maillageFissureParams, elementsDefaut, step, \
                                  mailleur="MeshGems"):
    """genereMaillageFissure"""
    maillageFissure = None
    return maillageFissure

  def setReferencesMaillageFissure(self):
    """setReferencesMaillageFissure"""
    referencesMaillageFissure = dict()
    return referencesMaillageFissure

  def mailleur2d3d(self):
    """Le mailleur : NETGEN ou MeshGems"""
    #mailleur = "MeshGems"
    mailleur = "NETGEN"
    return mailleur

# ---------------------------------------------------------------------------

  def executeProbleme(self, step=-1):
    """executeProbleme"""
    texte = " --- fissureGenerique.executeProbleme pour '{}', step = {}".format(self.nomCas,step)
    logging.info(texte)
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
    mailleur = self.mailleur2d3d()
    shapesFissure = self.genereShapeFissure(geometriesSaines, self.geomParams, self.shapeFissureParams, mailleur)
    if step == 3:
      return

    self.setParamMaillageFissure()
    elementsDefaut = self.genereZoneDefaut(geometriesSaines, maillagesSains, \
                                           shapesFissure, self.shapeFissureParams, self.maillageFissureParams)
    if step == 4:
      return

    self.maillageFissure = self.genereMaillageFissure(geometriesSaines, maillagesSains, \
                                                      shapesFissure, self.shapeFissureParams, self.maillageFissureParams, \
                                                      elementsDefaut, step, mailleur)

    self.setReferencesMaillageFissure()
    ok_maillage = getStatsMaillageFissure(self.maillageFissure, self.referencesMaillageFissure, self.maillageFissureParams)
    return ok_maillage
