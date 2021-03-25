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

import logging
from .geomsmesh import geompy

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
