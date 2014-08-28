# -*- coding: utf-8 -*-

import os
from blocFissure import gmu
from blocFissure.gmu import initLog
initLog.setDebug()
#initLog.setVerbose()

from blocFissure.gmu.casStandard import casStandard

dicoParams = dict(nomCas            = 'angleCube',
                  maillageSain      = os.path.join(gmu.pathBloc, 'materielCasTests/CubeAngle.med'),
                  brepFaceFissure   = os.path.join(gmu.pathBloc, "materielCasTests/CubeAngleFiss.brep"),
                  edgeFissIds       = [4],
                  lgInfluence       = 20,
                  meshBrep          = (5,10),
                  rayonPipe         = 5,
                  lenSegPipe        = 2.5,
                  nbSegRad          = 5,
                  nbSegCercle       = 32,
                  areteFaceFissure  = 10)

execInstance = casStandard(dicoParams)
