# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
"""Projection d'un point sur une courbe."""

import logging

from .geomsmesh import geompy

def projettePointSurCourbe(point, edge):
  """
  projection d'un point p sur une courbe c
  on suppose que la distance (c(u), p) passe par un minimum quand u varie entre 0 et 1
  et qu'elle presente pas de minimum local
  """
  #logging.debug("start")

  dist = list()
  nb_slices = 50
  delta = 1.0/float(nb_slices)
  for i_aux in range(nb_slices + 1):
    pti = geompy.MakeVertexOnCurve(edge, delta*float(i_aux))
    dpti = geompy.MinDistance(pti,point)
    dist.append((dpti,i_aux))
  dist.sort()

  #logging.debug("dist %s", dist)
  umin = delta*dist[0][1]
  umax = delta*dist[1][1]
  #umin = 0.0
  #umax = 1.0
  tol = 1.e-8
  pmin = geompy.MakeVertexOnCurve(edge, umin)
  pmax = geompy.MakeVertexOnCurve(edge, umax)
  dmin = geompy.MinDistance(pmin,point)
  dmax = geompy.MinDistance(pmax,point)
  dext = geompy.MinDistance(pmin,pmax)
  i_aux = 0
  while ( ( dext > tol ) and ( i_aux < 100 ) ):
    i_aux += 1
    utest = (umax + umin) / 2.0
    ptest = geompy.MakeVertexOnCurve(edge, utest)
    dtest = geompy.MinDistance(ptest,point)
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

  if ( abs(utest) < 1.e-7 ):
    utest = 0.0
  elif ( abs(1.0-utest) < 1.e-7 ):
    utest = 1.0
  logging.debug('u=%s, nbiter=%s dtest=%s dext=%s',utest,i_aux,dtest,dext)

  return utest
