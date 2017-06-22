# -*- coding: utf-8 -*-

import logging
from .geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- éliminer les doublons d'une liste de subshapes

def eliminateDoubles(obj, subshapes):
  """
  éliminer les doublons d'une liste de subshapes
  """
  idsubs = {}
  for sub in subshapes:
    subid = geompy.GetSubShapeID(obj, sub)
    if subid in list(idsubs.keys()):
      idsubs[subid].append(sub)
    else:
      idsubs[subid] = [sub]
  shortList = []
  for k, v in idsubs.items():
    shortList.append(v[0])
  logging.debug("shortList=%s", shortList)
  return shortList
