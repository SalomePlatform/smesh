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
"""Calcul de la position d'une shape par rapport à une face (dessus, dessous, sur la surface même)"""

import logging

from .geomsmesh import geompy

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
    if ( abs(angle) > 10 ):
      side = -1
  logging.debug("side %s distance %s", side, distance)

  return side
