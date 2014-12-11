# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
from sortFaces import sortFaces

def restreintFaceFissure(shapeDefaut, facesDefaut, pointInterne):
  """
  restriction de la face de fissure au domaine solide
  partition face fissure étendue par fillings
  """
  logging.info('start')
  partShapeDefaut = geompy.MakePartition([shapeDefaut], facesDefaut, [], [], geompy.ShapeType["FACE"], 0, [], 0)
  geomPublish(initLog.debug, partShapeDefaut, 'partShapeDefaut')
  facesPartShapeDefaut = geompy.ExtractShapes(partShapeDefaut, geompy.ShapeType["FACE"], False)
  if pointInterne is not None:
    distfaces = [(geompy.MinDistance(face,pointInterne), i, face) for i, face in enumerate(facesPartShapeDefaut)]
    distfaces.sort()
    logging.debug("selection de la face la plus proche du point interne, distance=%s",distfaces[0][0])
    facesPortFissure = distfaces[0][2]
  else:
    facesPartShapeDefautSorted, minSurf, maxSurf = sortFaces(facesPartShapeDefaut) # la face de fissure dans le volume doit être la plus grande
    logging.debug("surfaces faces fissure étendue, min %s, max %s", minSurf, maxSurf)
    facesPortFissure = facesPartShapeDefautSorted[-1]
  
  geomPublish(initLog.debug, facesPortFissure, "facesPortFissure")
  return facesPortFissure
