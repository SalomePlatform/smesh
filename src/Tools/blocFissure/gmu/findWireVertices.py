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
"""Trouver les vertices d'un wire"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

def findWireVertices(aWire, edges, getNormals=False):
  """Trouver les vertices d'un wire

  Calcul optionnel des tangentes. Attention à la tolérance qui peut être élevée (> 0.001)
  """
  logging.info("start")
  vertices = list()
  idsubs = dict()
  idnorm = dict()
  if getNormals:
    normals = list()

  for edge in edges:
    vert = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], False)
    vertices += vert
    if getNormals:
      vertex_0 = geompy.MakeVertexOnCurve(edge, 0.0)
      tangente_0 = geompy.MakeTangentOnCurve(edge, 0.0)
      tangente_1 = geompy.MakeTangentOnCurve(edge, 1.0)
      dist = geompy.MinDistance(vertex_0, vert[0])
      logging.debug("distance %s", dist)
      if dist < 1.e-2:
        normals += [tangente_0, tangente_1]
      else:
        normals += [tangente_1, tangente_0]

  for nro, sub in enumerate(vertices):
    subid = geompy.GetSubShapeID(aWire, sub)
    if subid in list(idsubs.keys()):
      idsubs[subid].append(sub)
    else:
      idsubs[subid] = [sub]
      name='vertex{}'.format(nro)
      geomPublishInFather(initLog.debug, aWire, sub, name)
      if getNormals:
        idnorm[subid] = normals[nro]
        name='norm{}'.format(nro)
        geomPublishInFather(initLog.debug, aWire, normals[nro], name)
  logging.debug("idsubs: %s", idsubs)

  return idsubs, idnorm
