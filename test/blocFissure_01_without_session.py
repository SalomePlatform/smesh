#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Tests de blocFissure ; en standalone

Cas tests basés sur les matériels :
. cubeAngle
. cubeFin

Copyright 2021-2024 CEA, EDF
Gérald NICOLAS
+33.1.78.19.43.52
"""

__revision__ = "V02.04"

import os
import sys

# Lancement des cas-tests
from salome.kernel import salome
salome.standalone()
salome.salome_init_without_session()

from blocFissure.CasTests.blocFissureTest import blocFissureTest
BLOCFISSURE_TEST = blocFissureTest(["cubeAngle", "cubeAngle_2","cubeCoin", "cubeMilieu", "cubeTransverse"])
MESSAGE_ERREUR = BLOCFISSURE_TEST.lancement()
#sys.stdout.write(MESSAGE_ERREUR)
del BLOCFISSURE_TEST

# Diagnostic
if MESSAGE_ERREUR:
  if ( "NOOK" in MESSAGE_ERREUR ):
    LAUX = MESSAGE_ERREUR.split()
    ERREUR = int(LAUX[LAUX.index("NOOK")+2])
  else:
    ERREUR = 0
else:
  MESSAGE_ERREUR = "Impossible de trouver le diagnostic de la procédure de tests."
  ERREUR = -1

if ERREUR:
  sys.stderr.write(MESSAGE_ERREUR)
  #raise Exception(MESSAGE_ERREUR)
  assert(False)

