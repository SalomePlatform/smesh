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
"""Trouver les vertices intermédiaires d'un wire"""

import logging

from .geomsmesh import geompy

from .findWireVertices import findWireVertices

# -----------------------------------------------------------------------------
# --- trouver les vertices intermediaires d'un wire

def findWireIntermediateVertices(aWire, getNormals=False):
  """Trouver les vertices d'un wire qui ne sont pas aux extremités

  Calcul optionnel des tangentes. Attention à la tolérance qui peut être élevée (> 0.001)
  """
  logging.info("start")
  edges = geompy.ExtractShapes(aWire, geompy.ShapeType["EDGE"], False)

  idsubs, idnorm = findWireVertices(aWire, edges, getNormals)

  shortList = list()
  shortNorm = list()
  for k_aux, sub in idsubs.items():
    if ( len(sub) > 1 ):
      shortList.append(sub[0])
      if getNormals:
        shortNorm.append(idnorm[k_aux])

  return shortList, shortNorm
