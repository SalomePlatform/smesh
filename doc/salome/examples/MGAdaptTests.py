#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Tests des adaptations par MGAdapt en standalone

Copyright 2021 EDF
Gérald NICOLAS
+33.1.78.19.43.52
"""

__revision__ = "V04.02"

#========================= Les imports - Début ===================================

import sys
import salome

import os
salome.standalone()
salome.salome_init()

l=list(os.environ.keys())
l.sort()
print (l)

import SMESH
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New()

#========================== Les imports - Fin ====================================

#========================= Paramétrage - Début ===================================
# 1. REPDATA = répertoire du cas
HOME = os.environ["HOME"]
REPDATA = os.path.join(HOME, "MAILLAGE", "TEST_REMAILLAGE", "MGAdapt_med_files")
PATH_SMESH = os.getenv("SMESH_ROOT_DIR")
print (PATH_SMESH)
PATH_SMESH = os.getenv("TEST_INSTALL_DIRECTORY")
print (PATH_SMESH)
REPDATA = os.path.join(PATH_SMESH, "MGAdapt_med_files")
print (REPDATA)
#
# 2. Repérage des données
D_DATA = dict()
D_DATA["01"] = "01"
D_DATA["02"] = "02"
D_DATA["03"] = "01"
D_DATA["04"] = "04"
D_DATA["05"] = "04"
D_DATA["06"] = "06"
D_DATA["07"] = "07"
D_DATA["08"] = "08"
D_DATA["10"] = "10"
D_DATA["11"] = "11"
#========================== Paramétrage - Fin ====================================

class MGAdaptTest (object):

  """Test de l'adaptation par MGAdapt

Options facultatives
********************
Le(s) nom du/des tests à passer. Si aucun n'est donné, tous les cas sont passés.
  """

# A. La base

  message_info = ""
  _verbose = 0
  _verbose_max = 0
  affiche_aide_globale = 0

# B. Les variables

  l_cas = None
  rep_test = None
  nro_cas = None
  cas = None

#=========================== Début de la méthode =================================

  def __init__ ( self, liste_option ):

    """Le constructeur de la classe MGAdaptTest"""

    self.l_cas = list()

    for option in liste_option :

      #print (option)
      saux = option.upper()
      #print (saux)
      if saux in ( "-H", "-HELP" ):
        self.affiche_aide_globale = 1
      elif saux == "-V" :
        self._verbose = 1
      elif saux == "-VMAX" :
        self._verbose = 1
        self._verbose_max = 1
      else :
        self.l_cas.append(option)

    if not self.l_cas:
      for cle in D_DATA.keys():
        self.l_cas.append(cle)
    self.l_cas.sort()

    if self._verbose_max:
      print ("Liste des cas : {}".format(self.l_cas))

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def __del__(self):
    """A la suppression de l'instance de classe"""
    if self._verbose_max:
      print ("Suppression de l'instance de la classe.")

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def _add_file_in (self, objet_adapt):
    """Ajout du fichier med d'entrée

Entrées/Sorties :
  :objet_adapt: l'objet du module
    """
    if self._verbose_max:
      print ("_add_file_in pour {}".format(self.nro_cas))

    ficmed = os.path.join (REPDATA, "test_{}.med".format(D_DATA[self.nro_cas]))
    if self._verbose_max:
      print ("Fichier {}".format(ficmed))
    if not os.path.isfile(ficmed):
      erreur = 1
      message = "Le fichier {} est inconnu.".format(ficmed)
    else:
      objet_adapt.setMEDFileIn(ficmed)
      erreur = 0
      message = ""

    return erreur, message

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def _add_file_out (self, objet_adapt):
    """Ajout du fichier de sortie

Entrées/Sorties :
  :objet_adapt: l'objet du module
    """
    if self._verbose_max:
      print ("_add_file_out pour {}".format(self.nro_cas))

    ficmed = os.path.join (REPDATA, "test_{}.adapt.tui.med".format(self.nro_cas))
    if os.path.isfile(ficmed):
      os.remove(ficmed)

    objet_adapt.setMEDFileOut(ficmed)

    if self._verbose:
      print (". Maillage adapté dans le fichier {}".format(ficmed))

    return

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def  _add_file_bkg (self, objet_adapt):
    """Ajout du fichier med de fond

Entrées/Sorties :
  :objet_adapt: l'objet du module
    """
    if self._verbose_max:
      print ("_add_file_bkg pour {}".format(self.nro_cas))

    ficmed = os.path.join (REPDATA, "test_{}_bg.med".format(D_DATA[self.nro_cas]))
    if self._verbose_max:
      print ("Fichier {}".format(ficmed))
    if not os.path.isfile(ficmed):
      erreur = 1
      message = "Le fichier {} est inconnu.".format(ficmed)
    else:
      objet_adapt.setMEDFileBackground(ficmed)
      erreur = 0
      message = ""

    return erreur, message

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def _hypo_creation (self, maptype, option, niveau=3):
    """Création d'une hypothèse

Entrées :
  :maptype: type de carte : "Local", "Background", "Constant"
  :option: nom du champ si "Local" ou "Background", valeur si "Constant"
  :niveau: niveau de verbosité

Sortie :
  :hypo: l'objet hypothèse
    """
    if self._verbose_max:
      print ("_hypo_creation pour {} avec {}".format(maptype,option))

    erreur = 0
    message = ""

    while not erreur :

      #--- Création de l'hypothèse ---
      hypo = smesh.CreateAdaptationHypothesis()

      # Type de données
      if ( maptype in ("Local", "Background", "Constant") ):
        hypo.setSizeMapType(maptype)
      else:
        message = "Le type de carte {} est inconnu.".format(maptype)
        erreur = 2
        break

      # Valeur
      if ( maptype in ("Local", "Background") ):
        hypo.setSizeMapFieldName(option)
      else:
        hypo.setConstantSize(option)

      # Verbosité
      self._hypo_verbose (hypo, niveau)

      break

    return erreur, message, hypo
#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def _hypo_verbose (self, hypo, niveau=10):
    """Débogage des hypothèses

Entrées :
  :niveau: niveau de verbosité

Entrées/Sorties :
  :hypo: l'objet hypothèse
    """

    hypo.setVerbosityLevel(niveau)

    if self._verbose_max:

      hypo.setPrintLogInFile(True)
      hypo.setKeepWorkingFiles(True)
      hypo.setRemoveOnSuccess(False)

    return
#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def _calcul (self, objet_adapt, hypo):
    """Calcul

Entrées :
  :objet_adapt: l'objet du module
  :hypo: l'objet hypothèse
    """

    #--- association de l'hypothese à l'objet de l'adaptation
    objet_adapt.AddHypothesis(hypo)

    #-- Calcul
    if self._verbose_max:
      print (hypo.getCommandToRun())
    erreur = objet_adapt.Compute(False)
    if erreur:
      message = "Echec dans l'adaptation."
    else:
      message = ""

    return erreur, message

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def _test_00 (self, objet_adapt):
    """Test générique

Entrées/Sorties :
  :objet_adapt: l'objet du module
    """

    erreur = 0
    message = ""

    while not erreur :

      #--- les fichiers MED ---#
      erreur, message = self._add_file_in (objet_adapt)
      if erreur:
        break
      self._add_file_out (objet_adapt)
      if self.nro_cas in ("02", "08"):
        erreur, message = self._add_file_bkg (objet_adapt)
        if erreur:
          break

      #--- Création de l'hypothèse ---
      if self.nro_cas in ("01", "04", "05", "06", "07", "10"):
        maptype = "Local"
        option = "TAILLE"
      elif self.nro_cas in ("02", "08"):
        maptype = "Background"
        option = "TAILLE"
      elif self.nro_cas in ("03",):
        maptype = "Constant"
        option = 0.5
      if self.nro_cas in ("11",):
        maptype = "Local"
        option = "Taille de maille"
      if self._verbose:
        niveau = 3
      elif self._verbose_max:
        niveau = 10
      else:
        niveau = 0
      erreur, message, hypo = self._hypo_creation(maptype, option, niveau)
      if erreur:
        break

      #-- Ajout des options
      if self.nro_cas in ("04", "06", "07", "08", "10", "11"):
        hypo.setTimeStepRankLast()
      elif self.nro_cas in ("05",):
        hypo.setTimeStepRank(1,1)

      # options facultatives
      if self.nro_cas in ("03",):
        hypo.setOptionValue("adaptation", "surface")

      #-- Calcul
      try :
        erreur, message = self._calcul (objet_adapt, hypo)
      except :
        erreur = 1871
        message = "Erreur dans le calcul par SMESH"

      break

    return erreur, message

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def _traitement_cas (self ):
    """Traitement d'un cas


Sorties :
  :erreur: code d'erreur
  :message: message d'erreur
    """

    nom_fonction = __name__ + "/_traitement_cas"
    blabla = "\nDans {} :".format(nom_fonction)

    if self._verbose_max:
      print (blabla)

# 1. Préalables

    erreur = 0
    message = ""
    if self._verbose:
      print ("Passage du cas '{}'".format(self.nro_cas))

    if ( self.nro_cas in ("01", "02", "03", "04", "05", "06", "07", "08", "10" , "11") ):
      objet_adapt = smesh.Adaptation('MG_Adapt')
      erreur, message = self._test_00 (objet_adapt)
      del objet_adapt

    else:
      erreur = 1
      message = "Le cas '{}' est inconnu.".format(self.nro_cas)

    return erreur, message

#===========================  Fin de la méthode ==================================

#=========================== Début de la méthode =================================

  def lancement (self):

    """Lancement

Sorties :
  :erreur: code d'erreur
  :message: message d'erreur
    """

    nom_fonction = __name__ + "/lancement"
    blabla = "\nDans {} :".format(nom_fonction)

    erreur = 0
    message = ""

    if self._verbose_max:
      print (blabla)

    for nom_cas in self.l_cas:
      self.nro_cas = nom_cas
      erreur_t, message_t = self._traitement_cas ()
      if erreur_t:
        erreur += 1
        message += "\nErreur n°{} pour le cas {} :\n".format(erreur_t,nom_cas)
        message += message_t

    if ( erreur and self._verbose_max ):
      print (blabla, message)

    return erreur, message

#===========================  Fin de la méthode ==================================

#==========================  Fin de la classe ====================================

#==================================================================================
# Lancement
#==================================================================================

if __name__ == "__main__" :

# 1. Options

  L_OPTIONS = list()
  #L_OPTIONS.append("-h")
  #L_OPTIONS.append("-v")
  #L_OPTIONS.append("-vmax")
  #L_OPTIONS.append("01")
  #L_OPTIONS.append("02")
  #L_OPTIONS.append("03")
  #L_OPTIONS.append("07")
  #L_OPTIONS.append("10")
  #L_OPTIONS.append("11")
  #L_OPTIONS.append("04")
  #L_OPTIONS.append("05")
  #L_OPTIONS.append("06")
  #L_OPTIONS.append("08")

# 2. Lancement de la classe

  #print ("L_OPTIONS :", L_OPTIONS)
  MGADAPT_TEST = MGAdaptTest(L_OPTIONS)
  if MGADAPT_TEST.affiche_aide_globale:
    sys.stdout.write(MGADAPT_TEST.__doc__+"\n")
  else:
    ERREUR, MESSAGE_ERREUR = MGADAPT_TEST.lancement()
    if ERREUR:
      sys.stdout.write(MGADAPT_TEST.__doc__+"\n")
      MESSAGE_ERREUR += "\n {} erreur(s)\n".format(ERREUR)
      sys.stderr.write(MESSAGE_ERREUR)
      raise Exception(MESSAGE_ERREUR)
      assert(False)

  del MGADAPT_TEST

  #sys.exit(0)
