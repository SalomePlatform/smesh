# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- subShapes communes à deux listes

def commonSubShapes(obj, sub1, sub2):
  """
  liste de subshapes communes
  """
  logging.info("start")
  idsub1 = {}
  subList = []
  for s in sub1:
    idsub1[geompy.GetSubShapeID(obj, s)] = s
  for s in sub2:
    idsub = geompy.GetSubShapeID(obj, s)
    if idsub in idsub1.keys():
      subList.append(s)
  logging.debug("subList=%s", subList)
  return subList
