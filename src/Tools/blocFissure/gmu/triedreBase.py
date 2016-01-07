# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

# --- origine et vecteurs de base

O = None
OX = None
OY = None
OZ = None

def triedreBase():
  """
  definitions globales du triedre de reference,
  objets partages par plusieurs methodes
  """
  global O, OX, OY, OZ
  
  if O == None:
    logging.info("start")
    O = geompy.MakeVertex(0, 0, 0)
    OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
    OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
    OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
  
    if not geompy.myStudy.FindObjectByName( 'OX', geompy.ComponentDataType() ):
      geomPublish(initLog.debug,  O, 'O' )
      geomPublish(initLog.debug,  OX, 'OX' )
      geomPublish(initLog.debug,  OY, 'OY' )
      geomPublish(initLog.debug,  OZ, 'OZ' )

  return O, OX, OY, OZ
