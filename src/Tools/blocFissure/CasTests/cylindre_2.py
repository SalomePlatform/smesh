# -*- coding: utf-8 -*-

import os
from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy, smesh

import math
import GEOM
import SALOMEDS
import SMESH
import logging

from cylindre import cylindre

from blocFissure.gmu.triedreBase import triedreBase
from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from blocFissure.gmu.creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from blocFissure.gmu.construitFissureGenerale import construitFissureGenerale

O, OX, OY, OZ = triedreBase()

class cylindre_2(cylindre):
  """
  problème de fissure plane sur cylindre hexa, fond de fissure complexe : polyline
  """
  nomProbleme = "cylindre2"

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams):
    logging.info("genereShapeFissure %s", self.nomCas)

    lgInfluence = shapeFissureParams['lgInfluence']

    shellFiss = geompy.ImportFile(os.path.join(gmu.pathBloc, "materielCasTests/FissInCylindre.brep"), "BREP")
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(fondFiss, [3])
    geompy.addToStudy( shellFiss, 'shellFiss' )
    geompy.addToStudyInFather( shellFiss, fondFiss, 'fondFiss' )


    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, 5 ,15)

    centre = None
    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict(Entity_Quad_Pyramid    = 1334,
                                          Entity_Quad_Triangle   = 1432,
                                          Entity_Quad_Edge       = 785,
                                          Entity_Quad_Penta      = 560,
                                          Entity_Quad_Hexa       = 19070,
                                          Entity_Node            = 114290,
                                          Entity_Quad_Tetra      = 19978,
                                          Entity_Quad_Quadrangle = 7424)

