# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- prolongation d'un wire par deux segments tangents

def prolongeWire(aWire, extrem, norms, long):
  """
  """
  logging.info("start")
  if geompy.NumberOfEdges(aWire) > 1:
    edges = geompy.ExtractShapes(aWire, geompy.ShapeType["EDGE"])
  else:
    edges = [aWire]
  for i, v1 in enumerate(extrem):
    exts = [geompy.MakeTranslationVectorDistance(v1, norms[i], l) for l in (-long, long)]
    dists = [(geompy.MinDistance(v, aWire), i , v) for i, v in enumerate(exts)]
    dists.sort()
    v2 = dists[-1][-1]
    #v2 = geompy.MakeTranslationVectorDistance(v1, norms[i], long)
    edge = geompy.MakeEdge(v1, v2)
    edges.append(edge)
    name = "extrem%d"%i
    #geompy.addToStudy(edge,name)
  wireProlonge = geompy.MakeWire(edges)
  geompy.addToStudy(wireProlonge, "wireProlonge")
  return wireProlonge
