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
"""Ajustement précis des points sur edgesPipeFissureExterneC"""

import logging

from .geomsmesh import geompy
from .findWireIntermediateVertices import findWireIntermediateVertices
from .projettePointSurCourbe import projettePointSurCourbe

def ajustePointsEdgePipeFissure(edgesPipeFissureExterneC, wirePipeFissureExterne, gptsdisks, idisklim):
  """Ajustement précis des points sur edgesPipeFissureExterneC"""
  logging.info('start')

  edgesPFE = geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False)
  verticesPFE, _ = findWireIntermediateVertices(wirePipeFissureExterne)  # vertices intermédiaires (des points en trop dans ptsInWireFissExtPipe)
  idiskmin = idisklim[0] + 1 # on ne prend pas le disque sur la peau, déjà ajusté
  idiskmax = idisklim[1]
  idiskint = list()

  for vtx in verticesPFE:
    distPtVt = list()
    for idisk in range(idiskmin, idiskmax):
      gptdsk = gptsdisks[idisk]
      point = gptdsk[0][-1]       # le point sur l'edge de la fissure externe au pipe
      distPtVt.append((geompy.MinDistance(point, vtx), idisk))
    distPtVt.sort()
    idiskint.append(distPtVt[0][1])
    gptsdisks[idiskint[-1]][0][-1] = vtx
    logging.debug("ajustement point sur edgePipeFissureExterne, vertex: %s %s", idiskint[-1], distPtVt[0][0])

  for idisk in range(idiskmin, idiskmax):
    if idisk in idiskint:
      break
    logging.debug("ajustement point sur edgePipeFissureExterne: %s", idisk)
    gptdsk = gptsdisks[idisk]
    point = gptdsk[0][-1]       # le point sur l'edge de la fissure externe au pipe
    distPtEd = [(geompy.MinDistance(point, edgePFE), k, edgePFE) for k, edgePFE in enumerate(edgesPFE)]
    distPtEd.sort()
    edgePFE = distPtEd[0][2]
    point_bis = projettePointSurCourbe(point, edgePFE)
    ptproj = geompy.MakeVertexOnCurve(edgePFE, point_bis)
    gptsdisks[idisk][0][-1] = ptproj

  return gptsdisks
