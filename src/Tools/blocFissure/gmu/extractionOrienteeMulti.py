# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

from whichSideMulti import whichSideMulti

# -----------------------------------------------------------------------------
# --- renvoie l'extraction des shapes d'un objet selon leur position par rapport à la face.

def extractionOrienteeMulti(faces, ifil, obj, centre, shapeType, tol, prefix=""):
  """
  renvoie l'extraction des shapes d'un objet selon leur position
  par rapport aux faces.
  shapeType in ["VERTEX", "EDGE", "FACE",...]
  """
  logging.info('start')
  trace = True
  shapesInside = []
  shapesOutside = []
  shapesOnside = []
  shapes = geompy.ExtractShapes(obj, geompy.ShapeType[shapeType], False)
  i=0
  j=0
  k=0
  prefix = prefix + shapeType
  for shape in shapes:
    side = whichSideMulti(faces, ifil, shape, centre, tol)
    if side == 1:
      shapesInside.append(shape)
      if trace:
        name = prefix + "_Inside%d"%i
        geomPublishInFather(initLog.debug, obj, shape, name)
      i+=1
    elif side == -1:
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

