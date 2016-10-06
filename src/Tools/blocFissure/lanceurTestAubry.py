# -*- coding: iso-8859-1 -*-

import sys
import salome

import os
from blocFissure import gmu
from blocFissure.gmu import initLog
#initLog.setDebug()
initLog.setVerbose()

from blocFissure.casStandard import casStandard

mesh = 'huehue.med'
crack = 'hue.brep'

dicoParams = dict(nomCas            = 'testAubry',
                  maillageSain      = '/local00/home/I48174/Bureau/{0}'.format(mesh),
                  brepFaceFissure   = '/local00/home/I48174/Bureau/{0}'.format(crack),
                  edgeFissIds       = [8],
                  lgInfluence       = 0.01,
                  meshBrep          = (0.0002,0.003),
                  rayonPipe         = 0.005,
                  lenSegPipe        = 0.0015,
                  nbSegRad          = 8,
                  nbSegCercle       = 18,
                  areteFaceFissure  = 0.0015)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
