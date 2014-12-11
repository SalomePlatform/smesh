# -*- coding: utf-8 -*-

import sys, traceback
import logging
from blocFissure import gmu
from blocFissure.gmu import initLog

# -----------------------------------------------------------------------------------------------
#initLog.setDebug()
initLog.setVerbose()
#initLog.setRelease()
#initLog.setPerfTests()

# ---tous les cas en sequence, ou les cas selectionnés ...
runall = True
if runall:
  torun = [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,]
else: #prob 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27   
  torun = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,]
# -----------------------------------------------------------------------------------------------

from blocFissure.gmu import geomsmesh
from blocFissure.gmu.casStandard import casStandard

problemes = []

cas=0
from blocFissure.CasTests.fissure_Coude import fissure_Coude
problemes.append(fissure_Coude(cas))

cas=1
from blocFissure.CasTests.faceGauche import faceGauche
problemes.append(faceGauche(cas))

cas=2
from blocFissure.CasTests.faceGauche_2 import faceGauche_2
problemes.append(faceGauche_2(cas))

cas=3
from blocFissure.CasTests.ellipse_1 import ellipse_1
problemes.append(ellipse_1(cas))

cas=4
from blocFissure.CasTests.fissure_Coude_4 import fissure_Coude_4
problemes.append(fissure_Coude_4(cas))

cas=5
from blocFissure.CasTests.cylindre import cylindre
problemes.append(cylindre(cas))

cas=6
from blocFissure.CasTests.cylindre_2 import cylindre_2
problemes.append(cylindre_2(cas))

cas=7
from blocFissure.CasTests.eprouvetteCourbe import eprouvetteCourbe
problemes.append(eprouvetteCourbe(cas))

cas=8
from blocFissure.CasTests.eprouvetteDroite import eprouvetteDroite
problemes.append(eprouvetteDroite(cas))

cas=9
from blocFissure.CasTests.eprouvetteDroite_2 import eprouvetteDroite_2
problemes.append(eprouvetteDroite_2(cas))

cas=10
from blocFissure.CasTests.cubeAngle import cubeAngle
problemes.append(cubeAngle(cas))

cas=11
from blocFissure.CasTests.fissureCoude_1 import fissureCoude_1
problemes.append(fissureCoude_1(cas))

cas=12
from blocFissure.CasTests.fissureCoude_2 import fissureCoude_2
problemes.append(fissureCoude_2(cas))

cas=13
from blocFissure.CasTests.fissureCoude_3 import fissureCoude_3
problemes.append(fissureCoude_3(cas))

cas=14
from blocFissure.CasTests.fissureCoude_4 import fissureCoude_4
problemes.append(fissureCoude_4(cas))

cas=15
from blocFissure.CasTests.fissureCoude_5 import fissureCoude_5
problemes.append(fissureCoude_5(cas))

cas=16
from blocFissure.CasTests.ellipse_2 import ellipse_2
problemes.append(ellipse_2(cas))

cas=17
from blocFissure.CasTests.cubeAngle2 import cubeAngle2
problemes.append(cubeAngle2(cas))

cas=18
from blocFissure.CasTests import disquePerce
problemes.append(casStandard(disquePerce.dicoParams, disquePerce.referencesMaillageFissure, cas))

cas=19
from blocFissure.CasTests.fissureCoude_6 import fissureCoude_6
problemes.append(fissureCoude_6(cas))

cas=20
from blocFissure.CasTests.fissureCoude_7 import fissureCoude_7
problemes.append(fissureCoude_7(cas))

cas=21
from blocFissure.CasTests.fissureCoude_8 import fissureCoude_8
problemes.append(fissureCoude_8(cas))

cas=22
from blocFissure.CasTests.fissureCoude_9 import fissureCoude_9
problemes.append(fissureCoude_9(cas))

cas=23
from blocFissure.CasTests.fissureCoude_10 import fissureCoude_10
problemes.append(fissureCoude_10(cas))

cas=24
from blocFissure.CasTests.vis_1 import vis_1
problemes.append(vis_1(cas))

cas=25
from blocFissure.CasTests import cubeCoin
problemes.append(casStandard(cubeCoin.dicoParams, cubeCoin.referencesMaillageFissure, cas))

cas=26
from blocFissure.CasTests import cubeMilieu
problemes.append(casStandard(cubeMilieu.dicoParams, cubeMilieu.referencesMaillageFissure, cas))

cas=27
from blocFissure.CasTests import cubeTransverse
problemes.append(casStandard(cubeTransverse.dicoParams, cubeTransverse.referencesMaillageFissure, cas))

  
for i in range(len(problemes)):
  if torun[i]:
    logging.critical("=== Execution cas %s", i)
    try:
      problemes[i].executeProbleme()
    except:
      traceback.print_exc()
    print "---------------------------------------------------------------------"
