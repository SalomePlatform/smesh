# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
from sortFaces import sortFaces
import traceback
from fissError import fissError

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
    try:
      facesPartShapeDefautSorted, minSurf, maxSurf = sortFaces(facesPartShapeDefaut) # la face de fissure dans le volume doit être la plus grande
    except:
      texte = "Restriction de la face de fissure au domaine solide impossible.<br>"
      texte += "Causes possibles :<ul>"
      texte += "<li>La face de fissure est tangente à la paroi solide."
      texte += "Elle doit déboucher franchement, sans que la surface dehors ne devienne plus grande que la surface dans le solide.</li>"
      texte += "<li>le prémaillage de la face de fissure est trop grossier, les mailles à enlever dans le maillage sain "
      texte += "n'ont pas toutes été détectées.</li></ul>"
      raise fissError(traceback.extract_stack(),texte)
    logging.debug("surfaces faces fissure étendue, min %s, max %s", minSurf, maxSurf)
    facesPortFissure = facesPartShapeDefautSorted[-1]
  
  geomPublish(initLog.debug, facesPortFissure, "facesPortFissure")
  return facesPortFissure
