# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

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
      nom = nom +"%d"%i
      logging.debug('nom: %s',nom)
      geompy.addToStudy(a, nom)

  shapeCompound = None
  if len(shapeList) > 0:
    shapeCompound =geompy.MakeCompound(shapeList)

  return shapeCompound
    
