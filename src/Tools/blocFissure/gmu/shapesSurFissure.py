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
"""Identification des shapes modifiées par la duplication des noeuds de la face fissure (d'un coté de la face)"""

import logging

import GEOM

from .geomsmesh import geompy

def shapesSurFissure(blocPartition, plane1, faceFissure, gencnt):
  """Identification des shapes modifiées par la duplication des noeuds de la face fissure (d'un coté de la face)"""

  logging.info('start')

  vertex = geompy.MakeVertexOnSurface(plane1, 0.5, 0.5)
  normal = geompy.GetNormal(plane1, vertex)
  extrusion = geompy.MakePrismVecH(plane1, normal, 100)

  sharedSolids = list()
  solids= geompy.GetShapesOnBox ( extrusion, blocPartition, geompy.ShapeType("SOLID"), GEOM.ST_ONIN )
  for solid in solids:
    sharedSolids += geompy.GetSharedShapes(faceFissure, solid, geompy.ShapeType["SOLID"])
  logging.debug("sharedSolids %s",sharedSolids)

  sharedFaces = list()
  faces= geompy.GetShapesOnBox ( extrusion, blocPartition, geompy.ShapeType("FACE"), GEOM.ST_ONIN )
  for face in faces:
    sharedFaces += geompy.GetSharedShapes(faceFissure, face, geompy.ShapeType["FACE"])
  logging.debug("sharedFaces %s",sharedFaces)

  sharedEdges = list()
  edges= geompy.GetShapesOnBox ( extrusion, blocPartition, geompy.ShapeType("EDGE"), GEOM.ST_ONIN )
  for edge in edges:
    if not edge.IsSame(gencnt):
      sharedEdges += geompy.GetSharedShapes(faceFissure, edge, geompy.ShapeType["EDGE"])
  logging.debug("sharedEdges %s",sharedEdges)

  return [ sharedSolids, sharedFaces, sharedEdges ]
