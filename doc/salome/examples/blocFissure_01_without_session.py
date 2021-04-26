#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Tests de blocFissure ; en standalone

Copyright 2021 EDF
Gérald NICOLAS
+33.1.78.19.43.52
"""

__revision__ = "V02.02"

import os
import sys

# Lancement des cas-tests
import salome
salome.standalone()
salome.salome_init()

from blocFissure.CasTests import blocFissureTest
BLOCFISSURE_TEST = blocFissureTest(["cubeAngle", "cubeAngle2", "cubeCoin", "cubeMilieu", "cubeTransverse"])
TEXTE = BLOCFISSURE_TEST.lancement()
del BLOCFISSURE_TEST

# Diagnostic
ERREUR = 0
if TEXTE:
  for LIGNE in TEXTE:
    #print (LIGNE[:-1])
    if ( "NOOK" in LIGNE ):
      MESSAGE_ERREUR = TEXTE
      ERREUR = int(LIGNE.split()[-1])
      break
else:
  MESSAGE_ERREUR = "Impossible de trouver le diagnostic de la procédure de tests."
  ERREUR = -1

if ERREUR:
  sys.stderr.write(MESSAGE_ERREUR)
  #raise Exception(MESSAGE_ERREUR)
  assert(False)
