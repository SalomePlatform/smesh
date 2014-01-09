# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- tri par volume de solides

def sortSolids(solidsToSort):
  """
  tri des solides par volume
  """
  logging.info('start')

  volSolids = [(geompy.BasicProperties(solid)[2], i, solid) for i, solid in enumerate(solidsToSort)]
  volSolids.sort()
  solidsSorted = [solid for vol, i, solid in volSolids]
  return solidsSorted, volSolids[0][0], volSolids[-1][0]

