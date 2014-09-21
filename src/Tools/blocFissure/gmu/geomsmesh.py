# -*- coding: utf-8 -*-

import logging
logging.info('start')
from initLog import getLogLevel

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(salome.myStudy)

logging.debug("initialisation de geompy et smesh OK")

def geomPublishDebug(aShape, aName):
  if getLogLevel() <= 1:
    geompy.addToStudy(aShape, aName)
    
def geomPublishDebugInFather(aFather, aShape, aName):
  if getLogLevel() <= 1:
    geompy.addToStudyInFather(aFather, aShape, aName)
    