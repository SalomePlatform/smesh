# -*- coding: utf-8 -*-

import sys
import salome

import os
from blocFissure import gmu
from blocFissure.gmu import initLog
#initLog.setDebug()
initLog.setVerbose()

from blocFissure.casStandard import casStandard

dicoParams = dict(nomCas            = 'casTestCoinTriple',
                  maillageSain      = '/local00/home/B27118/projets/blocFissure/blocFissure/materielCoinSymetrie/cubeTestCoinTriple.med',
                  brepFaceFissure   = "/local00/home/B27118/projets/blocFissure/blocFissure/materielCoinSymetrie/fissureTransverseTestCoinTriple.brep",
                  edgeFissIds       = [6],
                  lgInfluence       = 50,
                  meshBrep          = (5,10),
                  rayonPipe         = 10,
                  lenSegPipe        = 7,
                  nbSegRad          = 5,
                  nbSegCercle       = 10,
                  areteFaceFissure  = 10)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(1)
