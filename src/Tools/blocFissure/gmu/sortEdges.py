# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- tri par longueur d'edges

def sortEdges(edgesToSort):
  """
  tri des edges par longueur
  """
  logging.info('start')

  lenEdges = [(geompy.BasicProperties(edge)[0], i, edge) for i, edge in enumerate(edgesToSort)]
  lenEdges.sort()
  edgesSorted = [edge for length, i, edge in lenEdges]
  return edgesSorted, lenEdges[0][0], lenEdges[-1][0]

