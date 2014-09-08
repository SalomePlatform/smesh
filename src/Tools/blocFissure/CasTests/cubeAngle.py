# -*- coding: utf-8 -*-

import os
from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy, smesh

import math
import GEOM
import SALOMEDS
import SMESH
#import StdMeshers
#import GHS3DPlugin
#import NETGENPlugin
import logging

from blocFissure.gmu.fissureGenerique import fissureGenerique

from blocFissure.gmu.triedreBase import triedreBase
from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from blocFissure.gmu.creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from blocFissure.gmu.construitFissureGenerale import construitFissureGenerale

O, OX, OY, OZ = triedreBase()

class cubeAngle(fissureGenerique):
  """
  problème de fissure plane coupant 2 faces (angle), débouches normaux, objet plan
  """

  nomProbleme = "cubeAngle"

  # ---------------------------------------------------------------------------
  def genereMaillageSain(self, geometriesSaines, meshParams):
    logging.info("genereMaillageSain %s", self.nomCas)

    ([objetSain], status) = smesh.CreateMeshesFromMED(os.path.join(gmu.pathBloc, 'materielCasTests/CubeAngle.med'))
    smesh.SetName(objetSain.GetMesh(), 'objetSain')

    return [objetSain, True] # True : maillage hexa

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour méthode construitFissureGenerale
    lgInfluence : distance autour de la shape de fissure a remailler (A ajuster selon le maillage)
    rayonPipe   : le rayon du pile maillé en hexa autour du fond de fissure
    """
    logging.info("setParamShapeFissure %s", self.nomCas)
    self.shapeFissureParams = dict(lgInfluence = 20,
                                   rayonPipe   = 10)

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams):
    logging.info("genereShapeFissure %s", self.nomCas)

    lgInfluence = shapeFissureParams['lgInfluence']

    shellFiss = geompy.ImportFile(os.path.join(gmu.pathBloc, "materielCasTests/CubeAngleFiss.brep"), "BREP")
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(fondFiss, [4])
    geompy.addToStudy( shellFiss, 'shellFiss' )
    geompy.addToStudyInFather( shellFiss, fondFiss, 'fondFiss' )


    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, 5 ,10)

    centre = None
    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep           = '.',
                                      nomFicSain       = self.nomCas,
                                      nomFicFissure    = 'fissure_' + self.nomCas,
                                      nbsegRad         = 5,
                                      nbsegCercle      = 8,
                                      areteFaceFissure = 10)

  # ---------------------------------------------------------------------------
  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams):
    elementsDefaut = creeZoneDefautDansObjetSain(geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams)
    return elementsDefaut

  # ---------------------------------------------------------------------------
  def genereMaillageFissure(self, geometriesSaines, maillagesSains,
                            shapesFissure, shapeFissureParams,
                            maillageFissureParams, elementsDefaut, step):
    maillageFissure = construitFissureGenerale(maillagesSains,
                                               shapesFissure, shapeFissureParams,
                                               maillageFissureParams, elementsDefaut, step)
    return maillageFissure

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict(Entity_Quad_Pyramid    = 204,
                                          Entity_Quad_Triangle   = 336,
                                          Entity_Quad_Edge       = 278,
                                          Entity_Quad_Penta      = 96,
                                          Entity_Quad_Hexa       = 3651,
                                          Entity_Node            = 20490,
                                          Entity_Quad_Tetra      = 1877,
                                          Entity_Quad_Quadrangle = 1702)

