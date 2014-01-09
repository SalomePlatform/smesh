# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- calcul de la position d'une shape par rapport à une face (dessus, dessous, sur la surface même)

def whichSideVertex(face, vertex, tol = 1.e-3):
  """
  Position d'un vertex par rapport a une face (non nécessairement plane).
  renvoie 1 si le vertex est du coté de la normale à la face,
  -1 de l'autre coté, 0 si il est sur la face
  """
  #logging.info('start')
  side = 0
  distance = geompy.MinDistance(vertex, face)
  #logging.debug("    distance %s", distance)
  if distance > tol:
    projection = geompy.MakeProjection(vertex, face)
    normal = geompy.GetNormal(face, projection)
    vect = geompy.MakeVector(projection, vertex)
    angle = geompy.GetAngle(normal, vect)
    #logging.debug("  angle %s", angle)
    side = 1
    if abs(angle) > 10:
      side = -1
  logging.debug("side %s distance %s", side, distance)
  return side

