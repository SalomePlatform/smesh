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
"""ellipsoide defaut"""

import logging
import math

from .geomsmesh import geompy
from .triedreBase import triedreBase

O, OX, OY, OZ = triedreBase()

def ellipsoideDefaut(minRad,allonge):
  """Le bloc contenant la fissure est un ellipsoide construit centre a l'origine,
  contenant le tore elliptique de fissure

  @param minRad :petit rayon
  @param allonge :rapport grand rayon / petit rayon
  @return  ellipsoide (geomObject)
  """
  logging.info("start")
  boule = geompy.MakeSphereR(2)
  bouler = geompy.MakeRotation(boule, OY, math.pi/2.0)
  face = geompy.MakeFaceHW(100, 100, 3)
  boulepart = geompy.MakePartition([bouler], [face], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  solids = geompy.ExtractShapes(boulepart, geompy.ShapeType["SOLID"], True)
  solid0 = solids[0]
  for i in range(1,len(solids)):
    solid0 = geompy.MakeFuse(solid0, solids[i])
  ellipsoide = geompy.MakeScaleAlongAxes(solid0, O, minRad, minRad*(allonge+2.0)/2.0, minRad) # on limite l'allongement de l'ellipsoide
  #geompy.addToStudy( ellipsoide, 'ellipsoide' )
  return ellipsoide
