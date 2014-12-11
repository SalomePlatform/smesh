# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

from whichSide import whichSide

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

