# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- prolongation des segments extremité des polylines, pour la découpe

def prolongeVertices(vertices):
  """
  Prolongation des segments extremité d'une polyline définie par un vecteur de points.
  Chaque nouvelle extremité est obtenue par symétrie point du voisin de cette ancienne extrémité
  (symétrie de centre l'ancienne extrémite) : les segments extremes sont doublés.
  @param vertices : liste ordonnée des points (geomObject) de la polyline
  @return vertices : liste avec les deux extremités modifiées
  """
  logging.info("start")
  if len(vertices) < 2:
    return vertices
  v0 = vertices[0]
  v1 = vertices[1]
  m0 = geompy.MakeMirrorByPoint(v1, v0)
  ve = vertices[-1]
  vd = vertices[-2]
  m1 = geompy.MakeMirrorByPoint(vd, ve)
  vertices[0] = m0
  vertices[-1] = m1
  return vertices
