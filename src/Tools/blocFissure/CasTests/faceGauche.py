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

class faceGauche(fissureGenerique):
  """
  problème de fissure non plane, débouchante non normale
  """

  nomProbleme = "faceGauche"

  # ---------------------------------------------------------------------------
  def genereMaillageSain(self, geometriesSaines, meshParams):
    logging.info("genereMaillageSain %s", self.nomCas)

    ([objetSain], status) = smesh.CreateMeshesFromMED(os.path.join(gmu.pathBloc, "materielCasTests/faceGaucheSain.med"))
    smesh.SetName(objetSain.GetMesh(), 'objetSain')

    return [objetSain, True] # True : maillage hexa

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour méthode construitFissureGenerale
    lgInfluence : distance autour de la shape de fissure a remailler (A ajuster selon le maillage)
    rayonPipe   : le rayon du pile maillé en hexa autour du fond de fissure
    convexe     : optionnel True : la face est convexe (vue de l'exterieur) sert si on ne donne pas de point interne
    pointIn_x   : optionnel coordonnée x d'un point dans le solide sain (pour orienter la face)
    """
    logging.info("setParamShapeFissure %s", self.nomCas)
    self.shapeFissureParams = dict(lgInfluence = 300,
                                   rayonPipe   = 20)

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams):
    logging.info("genereShapeFissure %s", self.nomCas)

    lgInfluence = shapeFissureParams['lgInfluence']

    shellFiss = geompy.ImportFile(os.path.join(gmu.pathBloc, "materielCasTests/faceGaucheFiss.brep"), "BREP")
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(fondFiss, [6])
    geompy.addToStudy( shellFiss, 'shellFiss' )
    geompy.addToStudyInFather( shellFiss, fondFiss, 'fondFiss' )


    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, 5 ,50)

    centre = None
    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep           = '.',
                                      nomFicSain       = self.nomCas,
                                      nomFicFissure    = 'fissure_' + self.nomCas,
                                      nbsegRad         = 5,
                                      nbsegCercle      = 8,
                                      areteFaceFissure = 1000)

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
    self.referencesMaillageFissure = dict(Entity_Quad_Pyramid    = 1284,
                                          Entity_Quad_Triangle   = 2336,
                                          Entity_Quad_Edge       = 758,
                                          Entity_Quad_Penta      = 984,
                                          Entity_Quad_Hexa       = 6416,
                                          Entity_Node            = 85673,
                                          Entity_Quad_Tetra      = 35990,
                                          Entity_Quad_Quadrangle = 4285)

