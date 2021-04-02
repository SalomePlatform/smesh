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
"""Renvoie l'extraction des shapes d'un objet selon leur position par rapport à la fac"""

import logging

from .geomsmesh import geompy

from .whichSideMulti import whichSideMulti
from .extractionOrientee_a import extractionOrientee_a

def extractionOrienteeMulti(faces, ifil, obj, centre, shapeType, tol, prefix=""):
  """
  renvoie l'extraction des shapes d'un objet selon leur position
  par rapport aux faces.
  shapeType in ["VERTEX", "EDGE", "FACE",...]
  """
  logging.info('start')
  trace = True
  shapesInside = list()
  shapesOutside = list()
  shapesOnside = list()
  shapes = geompy.ExtractShapes(obj, geompy.ShapeType[shapeType], False)

  i_aux = 0
  j_aux = 0
  k_aux = 0
  prefix = prefix + shapeType
  for shape in shapes:
    side = whichSideMulti(faces, ifil, shape, centre, tol)
    i_aux, j_aux, k_aux = extractionOrientee_a(obj, shape, side, 1, \
                                               shapesInside, shapesOutside, shapesOnside, \
                                               i_aux, j_aux, k_aux, \
                                               trace, prefix)

  return [shapesInside, shapesOutside, shapesOnside]
