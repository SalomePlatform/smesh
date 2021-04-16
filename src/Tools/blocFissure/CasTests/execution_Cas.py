# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
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
"""Lancement des cas-tests de blocFissure"""

import logging
import os
import tempfile
import traceback

from blocFissure.gmu import initLog
from blocFissure.gmu.casStandard import casStandard

# -----------------------------------------------------------------------------------------------
LOGFILE = os.path.join(tempfile.gettempdir(),"blocFissure.log")
#initLog.setDebug(LOGFILE)     # debug = 10
initLog.setVerbose(LOGFILE)    # info = 20
#initLog.setRelease(LOGFILE)   # warning = 30
#initLog.setPerfTests(LOGFILE) # critical = 50
#initLog.setAlways(LOGFILE)    # critical = 50

# ---tous les cas en séquence, ou les cas sélectionnés ...
TORUNOK = [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0] # OK
#           0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28
#RUNALL = False
RUNALL = True
if RUNALL:
  TORUN =   [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
#             0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28
else:
  TORUNPB = list()
  for IAUX in TORUNOK:
    TORUNPB.append((IAUX+1)%2)
  print ("TORUN = {} # OK".format(TORUNOK))
  print ("TORUN = {} # PB".format(TORUNPB))
#                                                                                                    genereMateriel
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0] # aucun
  TORUN = [ 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # cubeAngle
  TORUN = [ 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # cubeFin
  TORUN = [ 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # decoupeCylindre
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # disque_perce + ellipse_disque
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # ellipse + fissureGauche2
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # ellipse_probleme + fissureGauche2
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # eprouvetteCourbe
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # eprouvetteDroite
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # fissureGauche
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] # fissureGauche2
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0] # vis
  TORUN = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1] # tube
#           0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28
  TORUN = [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0] #  OK
# -----------------------------------------------------------------------------------------------

def caract_l_problemes ():
  """Caractérisation des problèmes"""
  d_aux = dict()
  l_problemes = list()

  n_cas = 0
  # genereMateriel : cubeAngle
  from blocFissure.CasTests.cubeAngle import cubeAngle
  l_problemes.append(cubeAngle(n_cas))

  n_cas = 1
  # genereMateriel : cubeAngle
  from blocFissure.CasTests.cubeAngle2 import cubeAngle2
  l_problemes.append(cubeAngle2(n_cas))

  n_cas = 2
  # genereMateriel : cubeFin
  from blocFissure.CasTests import cubeCoin
  l_problemes.append(casStandard(cubeCoin.dicoParams, cubeCoin.referencesMaillageFissure, n_cas))
  d_aux[n_cas] = "cubeCoin"

  n_cas = 3
  # genereMateriel : cubeFin
  from blocFissure.CasTests import cubeMilieu
  l_problemes.append(casStandard(cubeMilieu.dicoParams, cubeMilieu.referencesMaillageFissure, n_cas))
  d_aux[n_cas] = "cubeMilieu"

  n_cas = 4
  # genereMateriel : cubeFin
  from blocFissure.CasTests import cubeTransverse
  l_problemes.append(casStandard(cubeTransverse.dicoParams, cubeTransverse.referencesMaillageFissure, n_cas))
  d_aux[n_cas] = "cubeTransverse"

  n_cas = 5
  # genereMateriel : decoupeCylindre
  from blocFissure.CasTests.cylindre import cylindre
  l_problemes.append(cylindre(n_cas))

  n_cas = 6
  # genereMateriel : decoupeCylindre
  from blocFissure.CasTests.cylindre_2 import cylindre_2
  l_problemes.append(cylindre_2(n_cas))

  n_cas = 7
  # genereMateriel : disque_perce
  # genereMateriel : ellipse_disque
  from blocFissure.CasTests import disquePerce
  l_problemes.append(casStandard(disquePerce.dicoParams, disquePerce.referencesMaillageFissure, n_cas))
  d_aux[n_cas] = "disquePerce"

  n_cas = 8
  # genereMateriel: ellipse
  # genereMateriel : fissureGauche2
  from blocFissure.CasTests.ellipse_1 import ellipse_1
  l_problemes.append(ellipse_1(n_cas))

  n_cas = 9
  # genereMateriel: ellipse_probleme
  # genereMateriel : fissureGauche2
  from blocFissure.CasTests.ellipse_2 import ellipse_2
  l_problemes.append(ellipse_2(n_cas))

  n_cas = 10
  # genereMateriel : eprouvetteCourbe
  from blocFissure.CasTests.eprouvetteCourbe import eprouvetteCourbe
  l_problemes.append(eprouvetteCourbe(n_cas))

  n_cas = 11
  # genereMateriel : eprouvetteDroite
  from blocFissure.CasTests.eprouvetteDroite import eprouvetteDroite
  l_problemes.append(eprouvetteDroite(n_cas))

  n_cas = 12
  # genereMateriel : eprouvetteDroite
  from blocFissure.CasTests.eprouvetteDroite_2 import eprouvetteDroite_2
  l_problemes.append(eprouvetteDroite_2(n_cas))

  n_cas = 13
  # genereMateriel : fissureGauche
  from blocFissure.CasTests.faceGauche import faceGauche
  l_problemes.append(faceGauche(n_cas))

  n_cas = 14
  # genereMateriel : fissureGauche2
  from blocFissure.CasTests.faceGauche_2 import faceGauche_2
  l_problemes.append(faceGauche_2(n_cas))

  n_cas = 15
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_1 import fissureCoude_1
  l_problemes.append(fissureCoude_1(n_cas))

  n_cas = 16
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_2 import fissureCoude_2
  l_problemes.append(fissureCoude_2(n_cas))

  n_cas = 17
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_3 import fissureCoude_3
  l_problemes.append(fissureCoude_3(n_cas))

  n_cas = 18
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_4 import fissureCoude_4
  l_problemes.append(fissureCoude_4(n_cas))

  n_cas = 19
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_5 import fissureCoude_5
  l_problemes.append(fissureCoude_5(n_cas))

  n_cas = 20
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_6 import fissureCoude_6
  l_problemes.append(fissureCoude_6(n_cas))

  n_cas = 21
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_7 import fissureCoude_7
  l_problemes.append(fissureCoude_7(n_cas))

  n_cas = 22
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_8 import fissureCoude_8
  l_problemes.append(fissureCoude_8(n_cas))

  n_cas = 23
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_9 import fissureCoude_9
  l_problemes.append(fissureCoude_9(n_cas))

  n_cas = 24
  # genereMateriel : aucun
  from blocFissure.CasTests.fissureCoude_10 import fissureCoude_10
  l_problemes.append(fissureCoude_10(n_cas))

  n_cas = 25
  # genereMateriel : aucun
  from blocFissure.CasTests.fissure_Coude import fissure_Coude
  l_problemes.append(fissure_Coude(n_cas))

  n_cas = 26
  # genereMateriel : aucun
  from blocFissure.CasTests.fissure_Coude_4 import fissure_Coude_4
  l_problemes.append(fissure_Coude_4(n_cas))

  n_cas = 27
  # genereMateriel : vis
  from blocFissure.CasTests.vis_1 import vis_1
  l_problemes.append(vis_1(n_cas))

  n_cas = 28
  # genereMateriel : tube
  from blocFissure.CasTests.tube import tube
  l_problemes.append(tube(n_cas))
  #"Mesh_13" has not been computed:
  #-  "algo2d_facePeau_0" failed on "FACE_12 to mesh". Error: Algorithm failed. NgException at Surface meshing: Problem in Surface mesh generation
  #-  "algo1d_cercle1_0" failed on EDGE #20. Error: Algorithm failed. Source elements don't cover totally the geometrical edge
  #-  "algo1d_cercle1_0" failed on EDGE #17. Error: Algorithm failed. Source elements don't cover totally the geometrical edge
  #-  "algo1d_cercle1_0" failed on EDGE #15. Error: Algorithm failed. Source elements don't cover totally the geometrical edge
  #-  "algo1d_cercle1_0" failed on EDGE #12. Error: Algorithm failed. Source elements don't cover totally the geometrical edge

  return l_problemes, d_aux
#=============================================================
def calcul_cas (n_cas, cas, d_aux, ligne):
  """Calcul d'un cas"""
  texte = ""
  if n_cas in d_aux:
    nom = d_aux[n_cas]
  else:
    nom = cas.nomProbleme
  texte_a = "\n=== Exécution du cas n° {}, '{}'".format(n_cas,nom)
  logging.critical(ligne+texte_a)

  try:
    ok_maillage = cas.executeProbleme()
  except:
    traceback.print_exc()
    ok_maillage = False
  if not ok_maillage:
    texte = "Problème avec le cas n° {}, '{}'\n".format(n_cas,nom)
  print(ligne)

  return ok_maillage, texte
#=============================================================
def calcul_tout (l_problemes, d_aux):
  """Calcul de tous les cas"""

  ligne = "---------------------------------------------------------------------"
  texte = ""
  nb_cas_ok = 0
  nb_cas_nook = 0
  for n_cas, cas in enumerate(l_problemes):
    #print ("Cas n° {}, '{}'".format(n_cas,cas.nomProbleme))
    if TORUN[n_cas]:
      ok_maillage, texte_a = calcul_cas (n_cas, cas, d_aux, ligne)
      texte += texte_a
      if ok_maillage:
        nb_cas_ok += 1
      else:
        nb_cas_nook += 1
      print(ligne)

  nb_cas = nb_cas_nook + nb_cas_ok
  if ( nb_cas > 1):
    if nb_cas_nook:
      texte += ". Nombre de cas_tests OK   : {}\n".format(nb_cas_ok)
      texte += ". Nombre de cas_tests NOOK : {}\n".format(nb_cas_nook)
    else:
      texte += "Les {} tests se sont bien passés.\n".format(nb_cas)
  print (texte+ligne)

  return
#=============================================================
#=============================================================

while True:

  L_PROBLEMES, D_AUX = caract_l_problemes()

  if ( len(L_PROBLEMES) != len(TORUN) ):
    TEXTE  = "\nNombre de problèmes définis  : {}\n".format(len(L_PROBLEMES))
    TEXTE += "Longueur de la liste 'TORUN' : {}\n".format(len(TORUN))
    TEXTE += "\t==> Incohérence de programmation à corriger dans {}".format(__name__)
    print (TEXTE)
    break

  calcul_tout (L_PROBLEMES, D_AUX)

  break
