# -*- coding: utf-8 -*-

import logging
logging.info('start')

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(salome.myStudy)

logging.debug("initialisation de geompy et smesh OK")
