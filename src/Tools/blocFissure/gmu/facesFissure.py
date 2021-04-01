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
"""Faces fissure dans et hors tore, et edges face hors tore"""

import logging

import GEOM

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

def facesFissure(blocp, faceFissure, extrusionDefaut, genint):
  """Extraction des faces de fissure dans et hors tore, des edges le long du tore et en paroi

  @param faceFissure : la face de fissure avec la partie dans le tore elliptique et la partie externe
  @return (facefissintore, facefissoutore, edgeint, edgeext)
  """
  logging.info('start')

  [face_0,face_1] = geompy.ExtractShapes(faceFissure, geompy.ShapeType["FACE"], True)
  ed0 = geompy.ExtractShapes(face_0, geompy.ShapeType["EDGE"], True)
  ed1 = geompy.ExtractShapes(face_1, geompy.ShapeType["EDGE"], True)
  if len(ed0) > len(ed1):
    facefissintore = face_0
    facefissoutore = face_1
  else:
    facefissintore = face_1
    facefissoutore = face_0

  geomPublishInFather(initLog.debug, faceFissure, facefissintore,'facefissintore')
  geomPublishInFather(initLog.debug, faceFissure, facefissoutore,'facefissoutore')

  edgeint = geompy.GetShapesOnShape(extrusionDefaut, facefissoutore, geompy.ShapeType["EDGE"], GEOM.ST_IN)
  edgeext = geompy.GetShapesOnShape(extrusionDefaut, facefissoutore, geompy.ShapeType["EDGE"], GEOM.ST_ON)

  for i_aux, edge in enumerate(edgeint):
    name = "edgeint_{}".format(i_aux)
    geomPublishInFather(initLog.debug, facefissoutore, edge,name)
  for i_aux, edge in enumerate(edgeext):
    name = "edgeext_{}".format(i_aux)
    geomPublishInFather(initLog.debug, facefissoutore, edge,name)

  reverext = list()
  if len(edgeext) > 1:
    vertices = geompy.ExtractShapes(genint, geompy.ShapeType["VERTEX"], False)
    for edge in edgeext:
      vertedge = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], False)
      if ((geompy.GetSubShapeID(blocp, vertedge[0]) == geompy.GetSubShapeID(blocp, vertices[0])) or
          (geompy.GetSubShapeID(blocp, vertedge[0]) == geompy.GetSubShapeID(blocp, vertices[1]))):
        reverext.append(0)
      else:
        reverext.append(1)

  return facefissintore, facefissoutore, edgeint, edgeext, reverext
