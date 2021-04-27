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
__revision__ = "V03.02"

"""Lancement des cas-tests de blocFissure

Temps par type de matériel :
. cubeAngle : 24s ["cubeAngle", "cubeAngle_2"])
. cubeFin : 42s ["cubeCoin", "cubeMilieu", "cubeTransverse"]
. decoupeCylindre : 8s ["cylindre", "cylindre_2"]
. disquePerce (+ ellipse_disque) : 9s ["disquePerce"]
. fissureGauche : 26s ["faceGauche"]
. fissureGauche_2 (+ ellipse + ellipse_probleme): 22s ["ellipse_1", "ellipse_2", "faceGauche_2"]
. eprouvetteCourbe : 22s ["eprouvetteCourbe"]
. eprouvetteDroite : 31s ["eprouvetteDroite", "eprouvetteDroite_2"]
. vis : 9s ["vis"]
. tube : ["tube"]
. sans matériel : 66s ["fissureCoude_1", "fissureCoude_2", "fissureCoude_3", "fissureCoude_4", "fissureCoude_5"]
. sans matériel : 69s ["fissureCoude_6", "fissureCoude_7", "fissureCoude_8", "fissureCoude_9", "fissureCoude_10"]
. sans matériel : 28s ["fissure_Coude", "fissure_Coude_4"]
"""

import logging
import os
import sys
import tempfile
import traceback

from blocFissure.gmu import initLog
from blocFissure.gmu.casStandard import casStandard

# -----------------------------------------------------------------------------------------------
LOGFILE = os.path.join(tempfile.gettempdir(),"blocFissure.log")
#initLog.setDebug(LOGFILE)     # debug = 10
#initLog.setVerbose(LOGFILE)   # info = 20
#initLog.setRelease(LOGFILE)   # warning = 30
#initLog.setPerfTests(LOGFILE) # critical = 50
#initLog.setAlways(LOGFILE)    # critical = 50

# Les cas qui marchent ...
TORUNOK = [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 ] # OK 19/04/2021
#           0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28
TORUNPB = list()
for IAUX in TORUNOK:
  TORUNPB.append((IAUX+1)%2)
print ("TORUN = {} # OK".format(TORUNOK))
print ("TORUN = {} # PB".format(TORUNPB))

# Numéro de référence associé à chaque problème défini par le nom de son fichier dans le répertoire CasTests
NREF_PB = dict()
NREF_PB["cubeAngle"] = 0
NREF_PB["cubeAngle_2"] = 1
NREF_PB["cubeCoin"] = 2
NREF_PB["cubeMilieu"] = 3
NREF_PB["cubeTransverse"] = 4
NREF_PB["cylindre"] = 5
NREF_PB["cylindre_2"] = 6
NREF_PB["disquePerce"] = 7
NREF_PB["ellipse_1"] = 8
NREF_PB["ellipse_2"] = 9
NREF_PB["eprouvetteCourbe"] = 10
NREF_PB["eprouvetteDroite"] = 11
NREF_PB["eprouvetteDroite_2"] = 12
NREF_PB["faceGauche"] = 13
NREF_PB["faceGauche_2"] = 14
NREF_PB["fissureCoude_1"] = 15
NREF_PB["fissureCoude_2"] = 16
NREF_PB["fissureCoude_3"] = 17
NREF_PB["fissureCoude_4"] = 18
NREF_PB["fissureCoude_5"] = 19
NREF_PB["fissureCoude_6"] = 20
NREF_PB["fissureCoude_7"] = 21
NREF_PB["fissureCoude_8"] = 22
NREF_PB["fissureCoude_9"] = 23
NREF_PB["fissureCoude_10"] = 24
NREF_PB["fissure_Coude"] = 25
NREF_PB["fissure_Coude_4"] = 26
NREF_PB["vis_1"] = 27
NREF_PB["tube"] = 28

#=========================  Début de la classe ===================================

class blocFissureTest (object):

  """Test de blocFissure

Options facultatives
********************
Le(s) numéro/nom du/des tests à passer. Si aucun n'est donné, tous les cas sont passés.
  """

# A. La base

  message_info = ""
  _verbose = 0
  _verbose_max = 0
  affiche_aide_globale = 0

# B. Les variables

  l_cas = None
  l_problemes = None
  d_nom_probleme = None
  l_materiels = None

#=========================== Début de la méthode =================================

  def __init__ ( self, liste_option ):

    """Le constructeur de la classe blocFissureTest"""

    self.l_cas = list()
    erreur = False

    for option in liste_option :

      #print (option, type(option))
      if isinstance(option, str):
        saux = option.upper()
      else:
        saux = option
      #print (saux)
      # Options générales de pilotage
      if saux in ( "-H", "-HELP" ):
        self.affiche_aide_globale = 1
      elif saux == "-V" :
        self._verbose = 1
      elif saux == "-VMAX" :
        self._verbose = 1
        self._verbose_max = 1

      else :
      # Options du traitement
        if isinstance(option, int):
          self.l_cas.append(option)
        elif ( option in NREF_PB ):
          self.l_cas.append(NREF_PB[option])
        else:
          print ("Option inconnue : {}".format(option))
          erreur = True

# Si aucun cas n'est donné, on prend tous ceux par défaut
    if not self.l_cas:
      for i_aux, valeur in enumerate(TORUNOK):
        if valeur:
          self.l_cas.append(i_aux)

    if erreur:
      self.l_cas = list()

    if self._verbose_max:
      print ("Liste des cas : {}".format(self.l_cas))

    if self._verbose_max:
      initLog.setVerbose(LOGFILE)   # info = 20

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def __del__(self):
    """A la suppression de l'instance de classe"""
    if self._verbose_max:
      print ("Suppression de l'instance de la classe.")

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def caract_l_problemes (self):
    """Caractérisation des problèmes"""
    self.d_nom_probleme = dict()
    self.l_problemes = list()
    self.l_materiels = list()

    n_cas = 0
    if n_cas in self.l_cas:
      from blocFissure.CasTests.cubeAngle import cubeAngle
      self.l_problemes.append(cubeAngle(n_cas))
      self.l_materiels.append("cubeAngle")

    n_cas = 1
    if n_cas in self.l_cas:
      from blocFissure.CasTests.cubeAngle_2 import cubeAngle_2
      self.l_problemes.append(cubeAngle_2(n_cas))
      self.l_materiels.append("cubeAngle")

    n_cas = 2
    if n_cas in self.l_cas:
      from blocFissure.CasTests import cubeCoin
      self.l_problemes.append(casStandard(cubeCoin.dicoParams, cubeCoin.referencesMaillageFissure, n_cas))
      self.d_nom_probleme[n_cas] = "cubeCoin"
      self.l_materiels.append("cubeFin")

    n_cas = 3
    if n_cas in self.l_cas:
      from blocFissure.CasTests import cubeMilieu
      self.l_problemes.append(casStandard(cubeMilieu.dicoParams, cubeMilieu.referencesMaillageFissure, n_cas))
      self.d_nom_probleme[n_cas] = "cubeMilieu"
      self.l_materiels.append("cubeFin")

    n_cas = 4
    if n_cas in self.l_cas:
      from blocFissure.CasTests import cubeTransverse
      self.l_problemes.append(casStandard(cubeTransverse.dicoParams, cubeTransverse.referencesMaillageFissure, n_cas))
      self.d_nom_probleme[n_cas] = "cubeTransverse"
      self.l_materiels.append("cubeFin")

    n_cas = 5
    if n_cas in self.l_cas:
      from blocFissure.CasTests.cylindre import cylindre
      self.l_problemes.append(cylindre(n_cas))
      self.l_materiels.append("decoupeCylindre")

    n_cas = 6
    if n_cas in self.l_cas:
      from blocFissure.CasTests.cylindre_2 import cylindre_2
      self.l_problemes.append(cylindre_2(n_cas))
      self.l_materiels.append("decoupeCylindre")

    n_cas = 7
    if n_cas in self.l_cas:
      from blocFissure.CasTests import disquePerce
      self.l_problemes.append(casStandard(disquePerce.dicoParams, disquePerce.referencesMaillageFissure, n_cas))
      self.d_nom_probleme[n_cas] = "disquePerce"
      self.l_materiels.append("disquePerce")
      self.l_materiels.append("ellipse_disque")

    n_cas = 8
    if n_cas in self.l_cas:
      from blocFissure.CasTests.ellipse_1 import ellipse_1
      self.l_problemes.append(ellipse_1(n_cas))
      self.l_materiels.append("ellipse")
      self.l_materiels.append("fissureGauche_2")

    n_cas = 9
    if n_cas in self.l_cas:
      from blocFissure.CasTests.ellipse_2 import ellipse_2
      self.l_problemes.append(ellipse_2(n_cas))
      self.l_materiels.append("ellipse_probleme")
      self.l_materiels.append("fissureGauche_2")

    n_cas = 10
    if n_cas in self.l_cas:
      from blocFissure.CasTests.eprouvetteCourbe import eprouvetteCourbe
      self.l_problemes.append(eprouvetteCourbe(n_cas))
      self.l_materiels.append("eprouvetteCourbe")

    n_cas = 11
    if n_cas in self.l_cas:
      from blocFissure.CasTests.eprouvetteDroite import eprouvetteDroite
      self.l_problemes.append(eprouvetteDroite(n_cas))
      self.l_materiels.append("eprouvetteDroite")

    n_cas = 12
    if n_cas in self.l_cas:
      from blocFissure.CasTests.eprouvetteDroite_2 import eprouvetteDroite_2
      self.l_problemes.append(eprouvetteDroite_2(n_cas))
      self.l_materiels.append("eprouvetteDroite")

    n_cas = 13
    if n_cas in self.l_cas:
      from blocFissure.CasTests.faceGauche import faceGauche
      self.l_problemes.append(faceGauche(n_cas))
      self.l_materiels.append("fissureGauche")

    n_cas = 14
    if n_cas in self.l_cas:
      from blocFissure.CasTests.faceGauche_2 import faceGauche_2
      self.l_problemes.append(faceGauche_2(n_cas))
      self.l_materiels.append("fissureGauche_2")

    n_cas = 15
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_1 import fissureCoude_1
      self.l_problemes.append(fissureCoude_1(n_cas))

    n_cas = 16
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_2 import fissureCoude_2
      self.l_problemes.append(fissureCoude_2(n_cas))

    n_cas = 17
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_3 import fissureCoude_3
      self.l_problemes.append(fissureCoude_3(n_cas))

    n_cas = 18
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_4 import fissureCoude_4
      self.l_problemes.append(fissureCoude_4(n_cas))

    n_cas = 19
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_5 import fissureCoude_5
      self.l_problemes.append(fissureCoude_5(n_cas))

    n_cas = 20
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_6 import fissureCoude_6
      self.l_problemes.append(fissureCoude_6(n_cas))

    n_cas = 21
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_7 import fissureCoude_7
      self.l_problemes.append(fissureCoude_7(n_cas))

    n_cas = 22
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_8 import fissureCoude_8
      self.l_problemes.append(fissureCoude_8(n_cas))

    n_cas = 23
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_9 import fissureCoude_9
      self.l_problemes.append(fissureCoude_9(n_cas))

    n_cas = 24
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissureCoude_10 import fissureCoude_10
      self.l_problemes.append(fissureCoude_10(n_cas))

    n_cas = 25
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissure_Coude import fissure_Coude
      self.l_problemes.append(fissure_Coude(n_cas))

    n_cas = 26
    if n_cas in self.l_cas:
      from blocFissure.CasTests.fissure_Coude_4 import fissure_Coude_4
      self.l_problemes.append(fissure_Coude_4(n_cas))

    n_cas = 27
    if n_cas in self.l_cas:
      # genereMateriel : vis
      from blocFissure.CasTests.vis_1 import vis_1
      self.l_problemes.append(vis_1(n_cas))
      self.l_materiels.append("vis")

    n_cas = 28
    if n_cas in self.l_cas:
      # genereMateriel : tube
      from blocFissure.CasTests.tube import tube
      self.l_problemes.append(tube(n_cas))
      self.l_materiels.append("tube")
      #"Mesh_13" has not been computed:
      #-  "algo2d_facePeau_0" failed on "FACE_12 to mesh". Error: Algorithm failed. NgException at Surface meshing: Problem in Surface mesh generation
      #-  "algo1d_cercle1_0" failed on EDGE #20. Error: Algorithm failed. Source elements don't cover totally the geometrical edge
      #-  "algo1d_cercle1_0" failed on EDGE #17. Error: Algorithm failed. Source elements don't cover totally the geometrical edge
      #-  "algo1d_cercle1_0" failed on EDGE #15. Error: Algorithm failed. Source elements don't cover totally the geometrical edge
      #-  "algo1d_cercle1_0" failed on EDGE #12. Error: Algorithm failed. Source elements don't cover totally the geometrical edge

    return

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def cree_materiels (self):
    """Création des matériels"""

    if "cubeAngle" in self.l_materiels:
      from blocFissure.materielCasTests import cubeAngle

    if "cubeFin" in self.l_materiels:
      from blocFissure.materielCasTests import cubeFin

    if "decoupeCylindre" in self.l_materiels:
      from blocFissure.materielCasTests import decoupeCylindre

    if "disquePerce" in self.l_materiels:
      from blocFissure.materielCasTests import disquePerce

    if "ellipse_disque" in self.l_materiels:
      from blocFissure.materielCasTests import ellipse_disque

    if "ellipse_probleme" in self.l_materiels:
      from blocFissure.materielCasTests import ellipse_probleme

    if "ellipse" in self.l_materiels:
      from blocFissure.materielCasTests import ellipse

    if "fissureGauche" in self.l_materiels:
      from blocFissure.materielCasTests import fissureGauche

    if "fissureGauche_2" in self.l_materiels:
      from blocFissure.materielCasTests import fissureGauche_2

    if "eprouvetteCourbe" in self.l_materiels:
      from blocFissure.materielCasTests import eprouvetteCourbe

    if "eprouvetteDroite" in self.l_materiels:
      from blocFissure.materielCasTests import eprouvetteDroite

    if "vis" in self.l_materiels:
      from blocFissure.materielCasTests import vis

    if "tube" in self.l_materiels:
      from blocFissure.materielCasTests import tube

    return

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def calcul_cas (self, n_cas, cas, ligne):
    """Calcul d'un cas"""
    texte = ""
    if n_cas in self.d_nom_probleme:
      nom = self.d_nom_probleme[n_cas]
    else:
      nom = cas.nomProbleme
    texte_a = "\n=== Exécution du cas n° {}, '{}'".format(NREF_PB[nom],nom)
    logging.critical(ligne+texte_a)

    try:
      ok_maillage = cas.executeProbleme()
    except:
      traceback.print_exc()
      ok_maillage = False
    if not ok_maillage:
      texte = "Problème avec le cas n° {}, '{}'\n".format(NREF_PB[nom],nom)
    print(ligne)

    return ok_maillage, texte

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def calcul_tout (self):
    """Calcul de tous les cas"""

    ligne = "---------------------------------------------------------------------"
    texte = "\n"
    nb_cas_ok = 0
    nb_cas_nook = 0
    for n_cas, cas in enumerate(self.l_problemes):
      #print ("Cas n° {}, '{}'".format(n_cas,cas.nomProbleme))
      ok_maillage, texte_a = self.calcul_cas (n_cas, cas, ligne)
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
    logging.critical(ligne+texte)

    return texte

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def lancement (self):
    """Lancement général"""

    # Tri des problèmes
    self.caract_l_problemes ()

    # Création des matériels
    self.cree_materiels ()

    # Calcul des fissures
    texte = self.calcul_tout ()

    return texte

#===========================  Fin de la méthode ==================================

#==========================  Fin de la classe ====================================

#==================================================================================
# Lancement
#==================================================================================

if __name__ == "__main__" :

# 1. Options

  L_OPTIONS = sys.argv[1:]
  L_OPTIONS.append("-v")

# 2. Lancement de la classe

  #print ("L_OPTIONS :", L_OPTIONS)
  BLOCFISSURE_TEST = blocFissureTest(L_OPTIONS)

  if BLOCFISSURE_TEST.affiche_aide_globale:
    sys.stdout.write(BLOCFISSURE_TEST.__doc__+"\n")
  else:
    TEXTE = BLOCFISSURE_TEST.lancement()
    sys.stdout.write(TEXTE)

  del BLOCFISSURE_TEST
