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
"""Opérateur de rotation translation d'un objet centré à l'origine"""

import logging
import math

from .geomsmesh import geompy
from .geomsmesh import geomPublish

from . import initLog

from .triedreBase import triedreBase

O, OX, OY, OZ = triedreBase()

def rotTrans(objet, orientation, point, normal, trace = False):
  """
  Déplacement par rotation translation d'un objet centré à l'origine, vers un point de la surface de la pièce saine
  dans laquelle on insère le défaut.
  @param objet : objet original centré à l'origine (geomObject)
  @param orientation : rotation selon OX de l'objet original (degrés)
  @param point : le point qui sera le centre de l'objet déplacé (geomObject), en général sur la surface de la pièce saine
  @param normal : la normale à la surface de la pièce saine au point central (geomObject)
  @return trans : objet transformé (geomObject)
  """
  logging.info("start")

  planXY = geompy.MakePlaneLCS(None, 2000, 1)
  projXY = geompy.MakeProjection(normal, planXY)

  [v_1,v_2] = geompy.ExtractShapes(projXY, geompy.ShapeType["VERTEX"], False)
  xyz1 = geompy.PointCoordinates(v_1)
  xyz2 = geompy.PointCoordinates(v_2)
  x = xyz2[0] - xyz1[0]
  y = xyz2[1] - xyz1[1]
  sinalpha = y / math.sqrt(x*x + y*y)
  cosalpha = x / math.sqrt(x*x + y*y)
  alpha = math.asin(sinalpha)
  if ( cosalpha < 0. ):
    alpha = math.pi -alpha

  beta = geompy.GetAngleRadians(OZ, normal)
  [v_1,v_2] = geompy.ExtractShapes(normal, geompy.ShapeType["VERTEX"], False)
  xyz1 = geompy.PointCoordinates(v_1)
  xyz2 = geompy.PointCoordinates(v_2)
  if ( (xyz2[2] - xyz1[2]) < 0 ):
    beta = math.pi -beta

  rot0 = geompy.MakeRotation(objet, OX, orientation*math.pi/180.0)
  rot1 = geompy.MakeRotation(rot0, OZ, alpha)
  axe2 = geompy.MakeRotation(OY, OZ, alpha)
  rot2 = geompy.MakeRotation(rot1, axe2, beta -math.pi/2.)
  logging.debug("alpha %f",alpha)
  logging.debug("beta %f",beta)
  if trace:
    geomPublish(initLog.debug,  rot1, 'rot1' )
    geomPublish(initLog.debug,  axe2, 'axe2' )
    geomPublish(initLog.debug,  rot2, 'rot2' )

  xyz = geompy.PointCoordinates(point)
  trans = geompy.MakeTranslation(rot2, xyz[0], xyz[1], xyz[2])

  return trans
