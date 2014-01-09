# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- transformation d'une liste de subshapes en une liste d'Id

def getSubshapeIds(obj, subshapes):
  """
  transformation d'une liste de subshapes en une liste d'Id
  """
  logging.debug("start")
  subshapesId = []
  for sub in subshapes:
    subshapesId.append(geompy.GetSubShapeID(obj, sub))
  logging.debug("subshapesId=%s", subshapesId)
  return subshapesId
