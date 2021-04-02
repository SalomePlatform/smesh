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
"""Crée zone géométrique défaut a partir d'un filling"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

def creeZoneDefautFilling(filling, shapeDefaut, lgExtrusion=50):
  """Construction CAO de la zone à remailler, quand on utilise un filling,
  après appel creeZoneDefautMaillage et quadranglesToShapeNoCorner

  @param filling : la CAO de la peau du défaut reconstituée
  @param shapeDefaut : objet géométrique représentant la fissure
  (selon les cas, un point central, ou une shape plus complexe,
  dont on ne garde que les vertices)
  @return (facesDefaut = filling, centreDefaut, normalDefaut, extrusionDefaut)
  """
  logging.info("start")

  trace = True
  facesDefaut = filling
  centreSphere = geompy.MakeCDG(shapeDefaut)
  geomPublish(initLog.debug, centreSphere, "cdg_defaut")
  centreDefaut = geompy.MakeProjection(centreSphere, filling)
  if trace:
    geomPublish(initLog.debug, centreDefaut, "centreDefaut")
  normalDefaut = geompy.GetNormal(filling, centreDefaut)
  if trace:
    geomPublish(initLog.debug, normalDefaut, "normalDefaut")
  extrusionDefaut = geompy.MakePrismVecH(filling, normalDefaut, -lgExtrusion)
  if trace:
    geomPublish(initLog.debug, extrusionDefaut, "extrusionDefaut")

  return facesDefaut, centreDefaut, normalDefaut, extrusionDefaut
