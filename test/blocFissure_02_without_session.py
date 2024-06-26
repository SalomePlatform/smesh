#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Tests de blocFissure ; en standalone

Cas tests basés sur les matériels :
. decoupeCylindre
. disque_perce
. fissureGauche
. fissureGauche_2
+ ellipse, ellipse_disque, ellipse_probleme

Copyright 2021-2024 CEA, EDF
Gérald NICOLAS
+33.1.78.19.43.52
"""

__revision__ = "V02.04"

import os
import sys

# Lancement des cas-tests
import salome
salome.standalone()
salome.salome_init_without_session()

from blocFissure.CasTests.blocFissureTest import blocFissureTest
BLOCFISSURE_TEST = blocFissureTest(["cylindre", "cylindre_2", "disquePerce", "faceGauche","ellipse_1", "ellipse_2"])
#BLOCFISSURE_TEST = blocFissureTest(["cylindre", "cylindre_2", "disquePerce", "faceGauche","ellipse_1", "ellipse_2", "faceGauche_2"])
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

