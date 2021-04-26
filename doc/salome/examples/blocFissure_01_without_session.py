#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Tests de blocFissure ; en standalone

Copyright 2021 EDF
Gérald NICOLAS
+33.1.78.19.43.52
"""

__revision__ = "V02.01"

import os
import tempfile
import sys

# Fichier de diagnostic
LOGFILE = os.path.join(tempfile.gettempdir(),"blocFissure.log")
if os.path.isfile(LOGFILE):
  os.remove(LOGFILE)

# Lancement des cas-tests
import salome
salome.standalone()
salome.salome_init()

from blocFissure.materielCasTests import genereMateriel

from blocFissure.CasTests import execution_Cas

# Diagnostic
ERREUR = 0
if os.path.isfile(LOGFILE):
  with open(LOGFILE, 'r') as FICHIER:
    LES_LIGNES = FICHIER.readlines()
  for LIGNE in LES_LIGNES:
    #print (LIGNE[:-1])
    if ( "NOOK" in LIGNE ):
      MESSAGE_ERREUR = LIGNE
      ERREUR = int(LIGNE.split()[-1])
      break
else:
  MESSAGE_ERREUR = "Impossible de trouver le fichier de diagnostic {}".format(LOGFILE)
  ERREUR = -1

if ERREUR:
  sys.stderr.write(MESSAGE_ERREUR)
  #raise Exception(MESSAGE_ERREUR)
  assert(False)
