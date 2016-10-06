# -*- coding: iso-8859-1 -*-

import sys
import salome

import os
from blocFissure import gmu
from blocFissure.gmu import initLog
#initLog.setDebug()
initLog.setVerbose()

from blocFissure.casStandard import casStandard

mesh = 'areteViveSoudure.med'
crack = 'fissureSoudureTest.brep'

dicoParams = dict(nomCas            = 'casTestCoinTriple',
                  maillageSain      = '/local00/home/I48174/Documents/soudure/essaiFissure/{0}'.format(mesh),
                  brepFaceFissure   = '/local00/home/I48174/Documents/soudure/essaiFissure/{0}'.format(crack),
                  edgeFissIds       = [4],
                  lgInfluence       = 30,
                  meshBrep          = (5,10),
                  rayonPipe         = 10,
                  lenSegPipe        = 7,
                  nbSegRad          = 5,
                  nbSegCercle       = 10,
                  areteFaceFissure  = 10)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
