# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from findWireIntermediateVertices import findWireIntermediateVertices
from projettePointSurCourbe import projettePointSurCourbe

def ajustePointsEdgePipeFissure(edgesPipeFissureExterneC, wirePipeFissureExterne, gptsdisks, idisklim):
  """
  ajustement precis des points sur edgesPipeFissureExterneC
  """
  logging.info('start')
  
  edgesPFE = geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False)
  verticesPFE = findWireIntermediateVertices(wirePipeFissureExterne)  # vertices intermédiaires (des points en trop dans ptsInWireFissExtPipe)
  idiskmin = idisklim[0] + 1 # on ne prend pas le disque sur la peau, déjà ajusté
  idiskmax = idisklim[1]     # on ne prend pas le disque sur la peau, déjà ajusté
  idiskint = []
  for vtx in verticesPFE:
    distPtVt = []
    for idisk in range(idiskmin, idiskmax):
      gptdsk = gptsdisks[idisk]
      pt = gptdsk[0][-1]       # le point sur l'edge de la fissure externe au pipe
      distPtVt.append((geompy.MinDistance(pt, vtx), idisk))
    distPtVt.sort()
    idiskint.append(distPtVt[0][1])
    gptsdisks[idiskint[-1]][0][-1] = vtx
    logging.debug("ajustement point sur edgePipeFissureExterne, vertex: %s %s", idiskint[-1], distPtVt[0][0])
  for idisk in range(idiskmin, idiskmax):
    if idisk in idiskint:
      break
    logging.debug("ajustement point sur edgePipeFissureExterne: %s", idisk)
    gptdsk = gptsdisks[idisk]
    pt = gptdsk[0][-1]       # le point sur l'edge de la fissure externe au pipe
    distPtEd = [(geompy.MinDistance(pt, edgePFE), k, edgePFE) for k, edgePFE in enumerate(edgesPFE)]
    distPtEd.sort()
    edgePFE = distPtEd[0][2]
    u = projettePointSurCourbe(pt, edgePFE)
    ptproj = geompy.MakeVertexOnCurve(edgePFE, u)
    gptsdisks[idisk][0][-1] = ptproj

  return gptsdisks