# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

def trouveEdgesFissPeau(facesInside, facesOnside, edgesPipeIn, edgesFondIn, partitionPeauFissFond, edgesFissExtPeau):
  """
  pour les faces de peau sans extremité débouchante de fissure, il faut recenser les edges de fissure sur la face de peau
  """
  logging.info('start')
  
  j = 0
  for face in facesInside:
    edgesPeauFis = []
    edgesPipeFis = []
    edgesPipeFnd = []
    try:
      edgesPeauFis = geompy.GetSharedShapesMulti([geompy.MakeCompound(facesOnside), face], geompy.ShapeType["EDGE"])
      edgesPipeFis = geompy.GetSharedShapesMulti([geompy.MakeCompound(edgesPipeIn), face], geompy.ShapeType["EDGE"])
      edgesPipeFnd = geompy.GetSharedShapesMulti([geompy.MakeCompound(edgesFondIn), face], geompy.ShapeType["EDGE"])
    except:
      pass
    if (len(edgesPeauFis) > 0) and (len(edgesPipeFis) > 0) and (len(edgesPipeFnd) == 0):
      edgesFissExtPeau.append(edgesPeauFis[0])
      name="edgesFissExtPeau%d"%j
      geomPublishInFather(initLog.debug,partitionPeauFissFond, edgesPeauFis[0], name)
      j += 1
  return edgesFissExtPeau