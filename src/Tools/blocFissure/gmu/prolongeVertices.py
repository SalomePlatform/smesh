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
"""Prolongation des segments extrémités des polylines, pour la découpe"""

import logging

from .geomsmesh import geompy

def prolongeVertices(vertices):
  """
  Prolongation des segments extremité d'une polyline définie par un vecteur de points.
  Chaque nouvelle extrémité est obtenue par symétrie point du voisin de cette ancienne extrémité
  (symétrie de centre l'ancienne extrémite) : les segments extrêmes sont doublés.
  @param vertices : liste ordonnée des points (geomObject) de la polyline
  @return vertices : liste avec les deux extremités modifiées
  """
  logging.info("start")
  if len(vertices) < 2:
    return vertices

  m_0 = geompy.MakeMirrorByPoint(vertices[1], vertices[0])

  m_1 = geompy.MakeMirrorByPoint(vertices[-2], vertices[-1])

  vertices[0] = m_0
  vertices[-1] = m_1

  return vertices
