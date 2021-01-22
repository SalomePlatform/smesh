# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

import sys, traceback
import logging
from blocFissure import gmu
from blocFissure.gmu import initLog

# -----------------------------------------------------------------------------------------------
#initLog.setDebug()
#initLog.setVerbose()
#initLog.setRelease()
#initLog.setPerfTests()

# ---tous les cas en séquence, ou les cas sélectionnés ...
runall = True
if runall:
  torun = [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
else: #prob 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27
  torun = [ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1] # pb
  torun = [ 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0] # OK
# -----------------------------------------------------------------------------------------------

from blocFissure.gmu import geomsmesh
from blocFissure.gmu.casStandard import casStandard

d_aux = dict()
problemes = list()

n_cas = 0
from blocFissure.CasTests.cubeAngle import cubeAngle
problemes.append(cubeAngle(n_cas))

n_cas += 1
from blocFissure.CasTests.cubeAngle2 import cubeAngle2
problemes.append(cubeAngle2(n_cas))

n_cas += 1
from blocFissure.CasTests import cubeCoin
problemes.append(casStandard(cubeCoin.dicoParams, cubeCoin.referencesMaillageFissure, n_cas))
d_aux[n_cas] = "cubeCoin"

n_cas += 1
from blocFissure.CasTests import cubeMilieu
problemes.append(casStandard(cubeMilieu.dicoParams, cubeMilieu.referencesMaillageFissure, n_cas))
d_aux[n_cas] = "cubeMilieu"

n_cas += 1
from blocFissure.CasTests import cubeTransverse
problemes.append(casStandard(cubeTransverse.dicoParams, cubeTransverse.referencesMaillageFissure, n_cas))
d_aux[n_cas] = "cubeTransverse"

n_cas += 1
from blocFissure.CasTests.cylindre import cylindre
problemes.append(cylindre(n_cas))

n_cas += 1
from blocFissure.CasTests.cylindre_2 import cylindre_2
problemes.append(cylindre_2(n_cas))

n_cas += 1
from blocFissure.CasTests import disquePerce
problemes.append(casStandard(disquePerce.dicoParams, disquePerce.referencesMaillageFissure, n_cas))
d_aux[n_cas] = "disquePerce"

n_cas += 1
from blocFissure.CasTests.ellipse_1 import ellipse_1
problemes.append(ellipse_1(n_cas))

n_cas += 1
from blocFissure.CasTests.ellipse_2 import ellipse_2
problemes.append(ellipse_2(n_cas))

n_cas += 1
from blocFissure.CasTests.eprouvetteCourbe import eprouvetteCourbe
problemes.append(eprouvetteCourbe(n_cas))

n_cas += 1
from blocFissure.CasTests.eprouvetteDroite import eprouvetteDroite
problemes.append(eprouvetteDroite(n_cas))

n_cas += 1
from blocFissure.CasTests.eprouvetteDroite_2 import eprouvetteDroite_2
problemes.append(eprouvetteDroite_2(n_cas))

n_cas += 1
from blocFissure.CasTests.faceGauche import faceGauche
problemes.append(faceGauche(n_cas))

n_cas += 1
from blocFissure.CasTests.faceGauche_2 import faceGauche_2
problemes.append(faceGauche_2(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_1 import fissureCoude_1
problemes.append(fissureCoude_1(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_10 import fissureCoude_10
problemes.append(fissureCoude_10(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_2 import fissureCoude_2
problemes.append(fissureCoude_2(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_3 import fissureCoude_3
problemes.append(fissureCoude_3(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_4 import fissureCoude_4
problemes.append(fissureCoude_4(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_5 import fissureCoude_5
problemes.append(fissureCoude_5(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_6 import fissureCoude_6
problemes.append(fissureCoude_6(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_7 import fissureCoude_7
problemes.append(fissureCoude_7(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_8 import fissureCoude_8
problemes.append(fissureCoude_8(n_cas))

n_cas += 1
from blocFissure.CasTests.fissureCoude_9 import fissureCoude_9
problemes.append(fissureCoude_9(n_cas))

n_cas += 1
from blocFissure.CasTests.fissure_Coude import fissure_Coude
problemes.append(fissure_Coude(n_cas))

n_cas += 1
from blocFissure.CasTests.fissure_Coude_4 import fissure_Coude_4
problemes.append(fissure_Coude_4(n_cas))

n_cas += 1
from blocFissure.CasTests.vis_1 import vis_1
problemes.append(vis_1(n_cas))

#=============================================================
while True:

  if ( len(problemes) != len(torun) ):
    texte  = "\nNombre de problèmes définis  : {}\n".format(len(problemes))
    texte += "Longueur de la liste 'torun' : {}\n".format(len(torun))
    texte += "\t==> Incohérence de programmation à corriger."
    print (texte)
    break

  ligne = "---------------------------------------------------------------------"
  texte = ""
  for n_cas, cas in enumerate(problemes):
    #print ("Cas n° {}, '{}'".format(n_cas,cas.nomProbleme))
    if torun[n_cas]:
      if n_cas in d_aux:
        nom = d_aux[n_cas]
      else:
        nom = cas.nomProbleme
      texte_a = "\n=== Exécution du cas n° {}, '{}'".format(n_cas,nom)
      logging.critical(ligne+texte_a)
      try:
        cas.executeProbleme()
      except:
        traceback.print_exc()
        texte += "Problème avec le cas n° {}, '{}'\n".format(n_cas,nom)
      print(ligne)

  if not texte:
    texte = "Tous les tests se sont bien passés.\n"
  print (texte+ligne)

  break

