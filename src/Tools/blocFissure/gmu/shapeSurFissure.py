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
"""construction d'une shape de dectection des éléments à modifier suite à la la duplication des noeuds de la face fissure (d'un coté de la face)"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublish

from . import initLog

def shapeSurFissure(facesFissure):
  """Normaliser les vecteurs et ponderer par les surfaces..."""
  logging.info('start')

  normal = None
  subIds = geompy.SubShapeAllIDs(facesFissure, geompy.ShapeType["FACE"])
  if ( len(subIds) > 1 ):
    logging.debug("plusieurs faces de fissure")
    faces = geompy.ExtractShapes(facesFissure, geompy.ShapeType["FACE"], False)
    extrusions = list()
    for face in faces:
      vertex = geompy.MakeVertexOnSurface(face, 0.5, 0.5)
      normal = geompy.GetNormal(face, vertex)
      extrusion = geompy.MakePrismVecH(face, normal, 100)
      extrusions.append(extrusion)
    extrusionFaceFissure = geompy.MakeCompound(extrusions)
  else:
    logging.debug("une seule face de fissure")
    face = facesFissure
    vertex = geompy.MakeVertexOnSurface(face, 0.5, 0.5)
    normal = geompy.GetNormal(face, vertex)
    extrusionFaceFissure = geompy.MakePrismVecH(facesFissure, normal, 100)

  geomPublish(initLog.debug, extrusionFaceFissure, "extrusionFaceFissure")

  return extrusionFaceFissure, normal
