# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

def compoundFromList(elements, nom=None):
  """
  
  """
  logging.debug('start')
  
  shapeList = []
  for a in elements:
    if not isinstance(a, list):
      shapeList.append(a)
    else:
      if a[0] is not None:
        shapeList.append(a[0])
      
  if nom is not None:
    for i,a in enumerate(shapeList):
      nomi = nom +"%d"%i
      logging.debug('nom: %s',nomi)
      geomPublish(initLog.debug, a, nomi)

  shapeCompound = None
  if len(shapeList) > 0:
    shapeCompound =geompy.MakeCompound(shapeList)
    nomc = "compound_%s"%nom
    geomPublish(initLog.debug, shapeCompound, nomc)
  return shapeCompound
    
