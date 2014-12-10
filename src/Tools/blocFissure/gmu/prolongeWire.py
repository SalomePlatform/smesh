# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from orderEdgesFromWire import orderEdgesFromWire

# -----------------------------------------------------------------------------
# --- prolongation d'un wire par deux segments tangents

def prolongeWire(aWire, extrem, norms, long):
  """
  """
  logging.info("start")
  if geompy.NumberOfEdges(aWire) > 1:
    edges = geompy.ExtractShapes(aWire, geompy.ShapeType["EDGE"])
    uneSeuleEdge = False
  else:
    edges = [aWire]
    uneSeuleEdge = True
  edgesBout = []
  for i, v1 in enumerate(extrem):
    exts = [geompy.MakeTranslationVectorDistance(v1, norms[i], l) for l in (-long, long)]
    dists = [(geompy.MinDistance(v, aWire), i , v) for i, v in enumerate(exts)]
    dists.sort()
    v2 = dists[-1][-1]
    #v2 = geompy.MakeTranslationVectorDistance(v1, norms[i], long)
    edge = geompy.MakeEdge(v1, v2)
    edges.append(edge)
    edgesBout.append(edge)
    name = "extrem%d"%i
    geompy.addToStudy(edge,name)
  try:
    wireProlonge = geompy.MakeWire(edges)
    geompy.addToStudy(wireProlonge, "wireProlonge")
  except:
    logging.warning("probleme MakeWire, approche pas a pas")
    if uneSeuleEdge:
      edgelist = [aWire]
      accessList = [0]
    else:
      edgelist, accessList = orderEdgesFromWire(aWire)
    edge1 = edgelist[accessList[0]]
    if geompy.MinDistance(edgesBout[0], edge1) < 1.e-4 :
      i0 = 0
      i1 = 1
    else:
      i0 = 1
      i1 = 0
    wireProlonge = edgesBout[i0]
    for i in range(len(edgelist)):
      wireProlonge = geompy.MakeWire([wireProlonge, edgelist[accessList[i]]])
      geompy.addToStudy(wireProlonge, "wireProlonge_%d"%i)
    wireProlonge = geompy.MakeWire([wireProlonge,edgesBout[i1]])
    geompy.addToStudy(wireProlonge, "wireNonProlonge")
    logging.warning("prolongation wire pas a pas OK")
  return wireProlonge
