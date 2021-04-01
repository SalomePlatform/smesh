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
"""Tore : aces toriques et volumes du tore"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

from .extractionOrientee import extractionOrientee
from .getSubshapeIds import getSubshapeIds

def facesVolumesToriques(tore, plan, facesDefaut):
  """
  Extraction des deux faces  et volumes du tore partitionné, qui suivent la génératrice elliptique.
  @param tore : le tore partitionné et coupé.
  @param plan : le plan de coupe
  @return (facetore1,facetore2) les 2 faces selon la génératrice
  """
  logging.info("start")

  centre = geompy.MakeVertexOnSurface(plan, 0.5, 0.5)
  normal = geompy.GetNormal(plan, centre)
  reference = geompy.MakeTranslationVector(centre, normal)

  [facesInPlan, facesOutPlan, _] = extractionOrientee(plan, tore, reference, "FACE", 1.e-2, "faceTorePlan_")
  facesIdInPlan = getSubshapeIds(tore, facesInPlan)
  facesIdOutPlan = getSubshapeIds(tore, facesOutPlan)

  [_, _, facesOnSide] = extractionOrientee(facesDefaut, tore, reference, "FACE", 1.e-2, "faceTorePeau_")
  facesIdOnSide = getSubshapeIds(tore, facesOnSide)

  facetore1 = None
  for i_aux, faceId in enumerate(facesIdInPlan):
    if faceId not in facesIdOnSide:
      facetore1 = facesInPlan[i_aux]
      break
  geomPublishInFather(initLog.debug, tore, facetore1, 'facetore1' )

  facetore2 = None
  for i_aux, faceId in enumerate(facesIdOutPlan):
    if faceId not in facesIdOnSide:
      facetore2 = facesOutPlan[i_aux]
      break
  geomPublishInFather(initLog.debug, tore, facetore2, 'facetore2' )

  [volumeTore1, volumeTore2] = geompy.ExtractShapes(tore, geompy.ShapeType["SOLID"], True)
  geomPublishInFather(initLog.debug, tore, volumeTore1, 'volumeTore1' )
  geomPublishInFather(initLog.debug, tore, volumeTore2, 'volumeTore2' )

  return facetore1, facetore2, volumeTore1, volumeTore2
