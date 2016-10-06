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

mesh = 'ARE_sain_lin_rotated.med'
crack = 'fissureExtrados.brep'

dicoParams = dict(nomCas            = 'fissTuyau',
                  maillageSain      = '/local00/home/I48174/Documents/soudure/essaiFissure/{0}'.format(mesh),
                  brepFaceFissure   = '/local00/home/I48174/Documents/soudure/essaiFissure/{0}'.format(crack),
                  edgeFissIds       = [4],
                  lgInfluence       = 12,
                  meshBrep          = (0.05, 2.0),
                  rayonPipe         = 1.0,
                  lenSegPipe        = 1, #9,
                  nbSegRad          = 8,
                  nbSegCercle       = 20,
                  areteFaceFissure  = 1.0)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(True)
