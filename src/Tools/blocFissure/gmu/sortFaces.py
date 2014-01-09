# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- tri par surface de faces

def sortFaces(facesToSort):
  """
  tri des faces par surface
  """
  logging.info('start')

  surFaces = [(geompy.BasicProperties(face)[1], i, face) for i, face in enumerate(facesToSort)]
  surFaces.sort()
  facesSorted = [face for surf, i, face in surFaces]
  return facesSorted, surFaces[0][0], surFaces[-1][0]

