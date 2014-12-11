# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

# -----------------------------------------------------------------------------
# --- faces fissure dans et hors tore, et edges face hors tore

def facesFissure(blocp, faceFissure, extrusionDefaut, genint):
  """
  extraction des faces de fissure dans et hors tore, des edges le long du tore et en paroi
  @param faceFissure : la face de fissure avec la partie dans le tore elliptique et la partie externe
  @return (facefissintore, facefissoutore, edgeint, edgeext)
  """
  logging.info('start')

  [f0,f1] = geompy.ExtractShapes(faceFissure, geompy.ShapeType["FACE"], True)
  ed0 = geompy.ExtractShapes(f0, geompy.ShapeType["EDGE"], True)
  ed1 = geompy.ExtractShapes(f1, geompy.ShapeType["EDGE"], True)
  if len(ed0) > len(ed1):
    facefissintore = f0
    facefissoutore = f1
  else:
    facefissintore = f1
    facefissoutore = f0

  geomPublishInFather(initLog.debug, faceFissure, facefissintore,'facefissintore')
  geomPublishInFather(initLog.debug, faceFissure, facefissoutore,'facefissoutore')

  edgeint = geompy.GetShapesOnShape(extrusionDefaut, facefissoutore, geompy.ShapeType["EDGE"], GEOM.ST_IN)
  edgeext = geompy.GetShapesOnShape(extrusionDefaut, facefissoutore, geompy.ShapeType["EDGE"], GEOM.ST_ON)

  for i in range(len(edgeint)):
    name = "edgeint_%d"%i
    geomPublishInFather(initLog.debug, facefissoutore, edgeint[i],name)
  for i in range(len(edgeext)):
    name = "edgeext_%d"%i
    geomPublishInFather(initLog.debug, facefissoutore, edgeext[i],name)

  reverext = []
  if len(edgeext) > 1:
    vertices = geompy.ExtractShapes(genint, geompy.ShapeType["VERTEX"], False)
    for i in range(len(edgeext)):
      vertedge = geompy.ExtractShapes(edgeext[i], geompy.ShapeType["VERTEX"], False)
      if ((geompy.GetSubShapeID(blocp, vertedge[0]) == geompy.GetSubShapeID(blocp, vertices[0])) or
          (geompy.GetSubShapeID(blocp, vertedge[0]) == geompy.GetSubShapeID(blocp, vertices[1]))):
        reverext.append(0)
      else:
        reverext.append(1)

  return facefissintore, facefissoutore, edgeint, edgeext, reverext
