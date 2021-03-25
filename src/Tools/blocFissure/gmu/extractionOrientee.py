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

import logging
from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog

from .whichSide import whichSide

# -----------------------------------------------------------------------------
# --- renvoie l'extraction des shapes d'un objet selon leur position par rapport à la face.

def extractionOrientee(face, obj, ref, shapeType, tol, prefix=""):
  """
  renvoie l'extraction des shapes d'un objet selon leur position
  par rapport à la face.
  shapeType in ["VERTEX", "EDGE", "FACE",...]
  """
  logging.info('start')
  trace = True
  sideRef = whichSide(face, ref)
  logging.debug("ref side %s", sideRef)
  shapesInside = []
  shapesOutside = []
  shapesOnside = []
  shapes = geompy.ExtractShapes(obj, geompy.ShapeType[shapeType], False)
  i=0
  j=0
  k=0
  prefix = prefix + shapeType
  for shape in shapes:
    side = whichSide(face, shape, tol)
    if side == sideRef:
      shapesInside.append(shape)
      if trace:
        name = prefix + "_Inside%d"%i
        geomPublishInFather(initLog.debug, obj, shape, name)
      i+=1
    elif side == -sideRef:
      shapesOutside.append(shape)
      if trace:
        name = prefix + "_Outside%d"%j
        geomPublishInFather(initLog.debug, obj, shape, name)
      j+=1
    elif side == 0:
      shapesOnside.append(shape)
      if trace:
        name = prefix + "_Onside%d"%k
        geomPublishInFather(initLog.debug, obj, shape, name)
      k+=1
    logging.debug("--- shape was %s", name)
  return [shapesInside, shapesOutside, shapesOnside]

