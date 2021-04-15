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
"""Partition volume sain et bloc, face du bloc recevant la fissure"""

import logging

import GEOM

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

from . import initLog

def partitionVolumeSain(volumeSain,boiteDefaut):
  """
  Partition du volume complet sur lequel porte le calcul par le cube qui contiendra le defaut
  @param volumeSain : volume complet (geomObject)
  @param boiteDefaut : cube qui contiendra le defaut, positionné dans l'espace (son centre doit être au voisinage
  immediat de la peau de l'objet sain: le tore elliptique debouche de paroi)
  @return (volumeSainPart, partieSaine, volDefaut, faceBloc) : volume complet partionné par le cube, partie saine,
  bloc du defaut (solide commun au cube et au volume complet), face du bloc defaut correspondant à la paroi.
  """
  logging.info("start")

  volumeSainPart = geompy.MakePartition([volumeSain], [boiteDefaut], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  [a,b] = geompy.ExtractShapes(volumeSainPart, geompy.ShapeType["SOLID"], True)
  volDefaut = geompy.GetInPlaceByHistory(volumeSainPart, boiteDefaut)
  if geompy.GetSubShapeID(volumeSainPart,b) == geompy.GetSubShapeID(volumeSainPart,volDefaut):
    partieSaine = a
  else:
    partieSaine = b
  faceBloc = geompy.GetShapesOnShapeAsCompound(volumeSain, volDefaut, geompy.ShapeType["FACE"], GEOM.ST_ON)

  geomPublish(initLog.debug,  volumeSainPart, 'volumeSainPart' )
  geomPublishInFather(initLog.debug, volumeSainPart, partieSaine, 'partieSaine' )
  geomPublishInFather(initLog.debug, volumeSainPart, volDefaut, 'volDefaut' )
  geomPublishInFather(initLog.debug, volDefaut, faceBloc, 'faceBloc' )
  return volumeSainPart, partieSaine, volDefaut, faceBloc
