# -*- coding: utf-8 -*-

""" FissureLauncher:
    Permet de lancer le script 'casStandard'
    en fonction des paramètres contenus dans 'dicoParams'. """

#import sys
import salome

#import os
#from blocFissure import gmu
from blocFissure.gmu import initLog
initLog.setDebug()
#initLog.setVerbose()

from blocFissure.casStandard import casStandard

mesh = 'maillageSainTuyauFEM.med'
crack = 'fissureQuartEllipse.brep'

dicoParams = dict(nomCas            = 'fissTuyau',
                  maillageSain      = '/local00/home/I48174/Bureau/supportMaillage/{0}'.format(mesh),
                  brepFaceFissure   = '/local00/home/I48174/Bureau/supportMaillage/{0}'.format(crack),
                  edgeFissIds       = [9],
                  lgInfluence       = 250,
                  meshBrep          = (10, 50),
                  rayonPipe         = 35,
                  lenSegPipe        = 7, #9,
                  nbSegRad          = 6,
                  nbSegCercle       = 30,
                  areteFaceFissure  = 20)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(True)