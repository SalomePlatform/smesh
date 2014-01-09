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

from blocFissure.gmu.initEtude import initEtude
from blocFissure.gmu.triedreBase import triedreBase
from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from blocFissure.gmu.creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from blocFissure.gmu.insereFissureGenerale import insereFissureGenerale

O, OX, OY, OZ = triedreBase()

class casStandard(fissureGenerique):
  """
  problème de fissure standard, défini par :
  - un maillage sain (hexaèdres),
  - une face géométrique de fissure, qui doit légèrement dépasser hors du volume maillé
  - les numéros d'arêtes (edges géométriques) correspondant au fond de fissure
  - les paramètres de maillage de la fissure
  """

  # ---------------------------------------------------------------------------
  def __init__ (self, dicoParams, references = None, numeroCas = 0):
    initEtude()
    self.references = references
    self.dicoParams = dicoParams
    if self.dicoParams.has_key('nomCas'):
      self.nomCas = self.dicoParams['nomCas']
    else:
      self.nomCas = 'casStandard'    
    self.numeroCas = numeroCas
    if self.numeroCas != 0:
      self.nomCas = self.nomProbleme +"_%d"%(self.numeroCas)
    else:
      self.nomProbleme = self.nomCas
    if self.dicoParams.has_key('lenSegPipe'):
      self.lenSegPipe = self.dicoParams['lenSegPipe']
    else:
      self.lenSegPipe =self.dicoParams['rayonPipe']
    if self.dicoParams.has_key('step'):
      step = self.dicoParams['step']
    else:
      step = -1 # exécuter toutes les étapes
    if self.numeroCas == 0: # valeur par défaut : exécution immédiate, sinon execution différée dans le cas d'une liste de problèmes
      self.executeProbleme(step)
    
  # ---------------------------------------------------------------------------
  def genereMaillageSain(self, geometriesSaines, meshParams):
    logging.info("genereMaillageSain %s", self.nomCas)

    ([objetSain], status) = smesh.CreateMeshesFromMED(self.dicoParams['maillageSain'])
    smesh.SetName(objetSain.GetMesh(), 'objetSain')

    return [objetSain, True] # True : maillage hexa

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour méthode insereFissureGenerale
    lgInfluence : distance autour de la shape de fissure a remailler (A ajuster selon le maillage)
    rayonPipe   : le rayon du pile maillé en hexa autour du fond de fissure
    convexe     : True : la face est convexe (vue de l'exterieur) sert si on ne donne pas de point interne
    pointIn_x   : optionnel : coordonnée x d'un point dans le solide sain (pour orienter la face - idem avec y,z)
    """
    logging.info("setParamShapeFissure %s", self.nomCas)
    if self.dicoParams.has_key('pointInterieur'):
      self.shapeFissureParams = dict(lgInfluence = self.dicoParams['lgInfluence'],
                                     rayonPipe   = self.dicoParams['rayonPipe'],
                                     lenSegPipe  = self.lenSegPipe,
                                     pointIn_x   = self.dicoParams['pointInterieur'][0],
                                     pointIn_y   = self.dicoParams['pointInterieur'][1],
                                     pointIn_z   = self.dicoParams['pointInterieur'][2])
    else:
      self.shapeFissureParams = dict(lgInfluence = self.dicoParams['lgInfluence'],
                                     rayonPipe   = self.dicoParams['rayonPipe'],
                                     lenSegPipe  = self.lenSegPipe)

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams):
    logging.info("genereShapeFissure %s", self.nomCas)

    lgInfluence = shapeFissureParams['lgInfluence']

    shellFiss = geompy.ImportFile( self.dicoParams['brepFaceFissure'], "BREP")
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(fondFiss, self.dicoParams['edgeFissIds'] )
    geompy.addToStudy( shellFiss, 'shellFiss' )
    geompy.addToStudyInFather( shellFiss, fondFiss, 'fondFiss' )


    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, self.dicoParams['meshBrep'][0] ,self.dicoParams['meshBrep'][1])

    centre = None
    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep           = '.',
                                      nomFicSain       = self.nomCas,
                                      nomFicFissure    = 'fissure_' + self.nomCas,
                                      nbsegRad         = self.dicoParams['nbSegRad'],
                                      nbsegCercle      = self.dicoParams['nbSegCercle'],
                                      areteFaceFissure = self.dicoParams['areteFaceFissure'])

  # ---------------------------------------------------------------------------
  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams):
    elementsDefaut = creeZoneDefautDansObjetSain(geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams)
    return elementsDefaut

  # ---------------------------------------------------------------------------
  def genereMaillageFissure(self, geometriesSaines, maillagesSains,
                            shapesFissure, shapeFissureParams,
                            maillageFissureParams, elementsDefaut, step):
    maillageFissure = insereFissureGenerale(maillagesSains,
                                            shapesFissure, shapeFissureParams,
                                            maillageFissureParams, elementsDefaut, step)
    return maillageFissure

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    if self.references is not None:
      self.referencesMaillageFissure = self.references
    else:
      self.referencesMaillageFissure = dict(Entity_Quad_Pyramid    = 0,
                                            Entity_Quad_Triangle   = 0,
                                            Entity_Quad_Edge       = 0,
                                            Entity_Quad_Penta      = 0,
                                            Entity_Quad_Hexa       = 0,
                                            Entity_Node            = 0,
                                            Entity_Quad_Tetra      = 0,
                                            Entity_Quad_Quadrangle = 0)

