# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- substract a list of subShapes from another

def substractSubShapes(obj, subs, toRemove):
  """
  liste de subshapes par difference
  """
  logging.info("start")
  idToremove = {}
  subList = []
  for s in toRemove:
    idToremove[geompy.GetSubShapeID(obj, s)] = s
  for s in subs:
    idsub = geompy.GetSubShapeID(obj, s)
    if idsub not in idToremove.keys():
      subList.append(s)
  logging.debug("subList=%s", subList)
  return subList
