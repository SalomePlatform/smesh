# -*- coding: utf-8 -*-

import logging
#logging.info('start')
from . import initLog

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New()

from salome.smesh import smeshBuilder
smesh = smeshBuilder.New()

# logging.debug("initialisation de geompy et smesh OK")

def geomPublish(level,aShape, aName):
  if initLog.getLogLevel() <= level:
    geompy.addToStudy(aShape, aName)
    
def geomPublishInFather(level, aFather, aShape, aName):
  if initLog.getLogLevel() <= level:
    geompy.addToStudyInFather(aFather, aShape, aName)
    