# -*- coding: utf-8 -*-
# Copyright (C) 2014-2019  CEA/DEN, EDF R&D
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
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog

# -----------------------------------------------------------------------------
# --- trouver les vertices extremites d'un wire

def findWireEndVertices(aWire, getNormals=False):
  """
  trouver les vertices extremites d'un wire
  calcul optionnel des tangentes. Attention à la tolérance qui peut être élevée (> 0.001)
  """
  logging.info("start")
  if geompy.NumberOfEdges(aWire) > 1:
    edges = geompy.ExtractShapes(aWire, geompy.ShapeType["EDGE"], False)
  else:
    edges = [aWire]
  vertices = []
  idsubs = {}
  shortList = []
  if getNormals:
    normals = []
    idnorm = {}
    shortNorm = []
  for edge in edges:
    vert = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], False)
    vertices += vert
    if getNormals:
      v0 = geompy.MakeVertexOnCurve(edge, 0.0)
      n0 = geompy.MakeTangentOnCurve(edge, 0.0)
      v1 = geompy.MakeVertexOnCurve(edge, 1.0)
      n1 = geompy.MakeTangentOnCurve(edge, 1.0)
      dist = geompy.MinDistance(v0, vert[0])
      logging.debug("distance %s", dist)
      if dist < 1.e-2:
        normals += [n0, n1]
      else:
        normals += [n1, n0]
  for i, sub in enumerate(vertices):
    subid = geompy.GetSubShapeID(aWire, sub)
    if subid in list(idsubs.keys()):
      idsubs[subid].append(sub)
    else:
      idsubs[subid] = [sub]
      name='vertex%d'%i
      geomPublishInFather(initLog.debug, aWire, sub, name)
      if getNormals:
        idnorm[subid] = normals[i]
        name='norm%d'%i
        geomPublishInFather(initLog.debug, aWire, normals[i], name)
  logging.debug("idsubs: %s", idsubs)
  for k, v in idsubs.items():
    if len(v) == 1:
      shortList.append(v[0])
      if getNormals:
        shortNorm.append(idnorm[k])
  if getNormals:
    return shortList, shortNorm
  else:
    return shortList

