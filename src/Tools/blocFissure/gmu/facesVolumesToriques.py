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

import logging
from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog
from .extractionOrientee import extractionOrientee
from .getSubshapeIds import getSubshapeIds

# -----------------------------------------------------------------------------
# --- TORE
# --- faces toriques  et volumes du tore

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

  [facesInPlan, facesOutPlan, facesOnPlan] = extractionOrientee(plan, tore, reference, "FACE", 1.e-2, "faceTorePlan_")
  [facesInSide, facesOutSide, facesOnSide] = extractionOrientee(facesDefaut, tore, reference, "FACE", 1.e-2, "faceTorePeau_")
  facesIdInPlan = getSubshapeIds(tore, facesInPlan)
  facesIdOutPlan = getSubshapeIds(tore, facesOutPlan)
  facesIdOnSide = getSubshapeIds(tore, facesOnSide)
  facesIdInSide = getSubshapeIds(tore, facesInSide)
  facesIdOutSide = getSubshapeIds(tore, facesOutSide)
  #facesIdInOutSide = facesIdInSide + facesIdOutSide
  facetore1 = None
  faceTore2 = None
  for i, faceId in enumerate(facesIdInPlan):
    if faceId not in facesIdOnSide:
      facetore1 = facesInPlan[i]
      break
  for i, faceId in enumerate(facesIdOutPlan):
    if faceId not in facesIdOnSide:
      facetore2 = facesOutPlan[i]
      break

  #[facetore1,facetore2] = geompy.GetShapesOnShape(pipe0, tore, geompy.ShapeType["FACE"], GEOM.ST_ON)

  geomPublishInFather(initLog.debug, tore, facetore1, 'facetore1' )
  geomPublishInFather(initLog.debug, tore, facetore2, 'facetore2' )

  [volumeTore1, volumeTore2] = geompy.ExtractShapes(tore, geompy.ShapeType["SOLID"], True)
  geomPublishInFather(initLog.debug, tore, volumeTore1, 'volumeTore1' )
  geomPublishInFather(initLog.debug, tore, volumeTore2, 'volumeTore2' )

  return facetore1, facetore2, volumeTore1, volumeTore2
