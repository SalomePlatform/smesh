# -*- coding: utf-8 -*-

from geomsmesh import geompy
import logging
import math

# -----------------------------------------------------------------------------
# --- projection d'un point sur une courbe.

def projettePointSurCourbe(pt, edge):
  """
  projection d'un point p sur une courbe c
  on suppose que la distance (c(u), p) passe par un minimum quand u varie entre 0 et 1
  et qu'elle presente pas de minimum local 
  """
  #logging.debug("start")

  dist = []
  nbSlices = 50
  du = 1.0/nbSlices
  for i in range(nbSlices + 1):
    p = geompy.MakeVertexOnCurve(edge, du*i)
    d = geompy.MinDistance(p,pt)
    dist.append((d,i))
  dist.sort()
  #logging.debug("dist %s", dist)
  umin = du*dist[0][1]
  umax = du*dist[1][1]
  #umin = 0.0
  #umax = 1.0
  tol = 1.e-8
  pmin = geompy.MakeVertexOnCurve(edge, umin)
  pmax = geompy.MakeVertexOnCurve(edge, umax)
  dmin = geompy.MinDistance(pmin,pt)
  dmax = geompy.MinDistance(pmax,pt)
  dext = geompy.MinDistance(pmin,pmax)
  i=0
  while dext > tol and i < 100 :
    i = i+1
    utest = (umax + umin) / 2.0
    ptest = geompy.MakeVertexOnCurve(edge, utest)   
    dtest = geompy.MinDistance(ptest,pt)
    if dmin < dmax:
      umax = utest
      pmax = ptest
      dmax = dtest
    else:
      umin = utest
      pmin = ptest
      dmin = dtest
    dext = geompy.MinDistance(pmin,pmax)
    #logging.debug('umin=%s umax=%s dmin=%s dmax=%s dtest=%s dext=%s', umin,umax,dmin,dmax,dtest,dext)
  if abs(utest) < 1.e-7:
    utest = 0.0
  if abs(1.0-utest) < 1.e-7:
    utest = 1.0
  logging.debug('u=%s, nbiter=%s dtest=%s dext=%s',utest,i,dtest,dext)
  return utest
    