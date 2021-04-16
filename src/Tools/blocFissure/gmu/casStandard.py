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
"""Cas standard"""

import os
import logging

from .geomsmesh import geompy, smesh
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

from . import initLog

from .fissureGenerique import fissureGenerique

from .initEtude import initEtude
from .triedreBase import triedreBase
from .genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from .creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from .construitFissureGenerale import construitFissureGenerale
from .putName import putName

O, OX, OY, OZ = triedreBase()

class casStandard(fissureGenerique):
  """problème de fissure standard, défini par :

  - un maillage sain (hexaèdres),
  - une face géométrique de fissure, qui doit légèrement dépasser hors du volume maillé
  - les noms des groupes d'arêtes ou leurs numéros d'arêtes (edges au sens de GEOM) correspondant au fond de fissure
  - les paramètres de maillage de la fissure
  """
  referencesMaillageFissure = None

  # ---------------------------------------------------------------------------
  def __init__ (self, dicoParams, references = None, numeroCas = None):
    initEtude()
    self.references = references
    self.dicoParams = dicoParams
    self.numeroCas = numeroCas

    if 'nomProbleme' in self.dicoParams:
      self.nomProbleme = self.dicoParams['nomProbleme']

    if 'nomCas' in self.dicoParams:
      self.nomCas = self.dicoParams['nomCas']
    elif 'nomres' in self.dicoParams:
      self.nomCas = os.path.splitext(os.path.split(self.dicoParams['nomres'])[1])[0]
    elif ( self.numeroCas is not None ):
      self.nomCas = self.nomProbleme +"_%d"%(self.numeroCas)
    else:
      self.nomCas = 'casStandard'

    if 'reptrav' in self.dicoParams:
      self.reptrav = self.dicoParams['reptrav']
    else:
      self.reptrav = os.curdir

    if 'lenSegPipe' in self.dicoParams:
      self.lenSegPipe = self.dicoParams['lenSegPipe']
    else:
      self.lenSegPipe = self.dicoParams['rayonPipe']

    if 'step' in self.dicoParams:
      step = self.dicoParams['step']
    else:
      step = -1 # exécuter toutes les étapes

    if 'aretesVives' not in self.dicoParams:
      self.dicoParams['aretesVives'] = 0

    # valeur par défaut : exécution immédiate, sinon execution différée dans le cas d'une liste de problèmes
    if ( self.numeroCas is None ):
      self.executeProbleme(step)

  # ---------------------------------------------------------------------------
  def genereMaillageSain(self, geometriesSaines, meshParams):
    logging.info("genereMaillageSain %s", self.nomCas)

    ([objetSain], _) = smesh.CreateMeshesFromMED(self.dicoParams['maillageSain'])
    putName(objetSain.GetMesh(), objetSain.GetName(), i_pref=self.numeroCas)

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
    if 'pointInterieur' in self.dicoParams:
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
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams, \
                                mailleur="MeshGems"):

    lgInfluence = shapeFissureParams['lgInfluence']

#   Contrôle de 'brepFaceFissure' pour les anciennes versions
    if ( 'brepFaceFissure' in self.dicoParams ):
      self.dicoParams['CAOFaceFissure'] = self.dicoParams['brepFaceFissure']
    cao_file = self.dicoParams['CAOFaceFissure']
    suffix = os.path.basename(cao_file).split(".")[-1]
    if ( suffix.upper() == "BREP" ):
      shellFiss = geompy.ImportBREP(cao_file)
    elif ( suffix.upper() == "XAO" ):
      (_, shellFiss, _, l_groups, _) = geompy.ImportXAO(cao_file)
    fondFiss = geompy.CreateGroup(shellFiss, geompy.ShapeType["EDGE"])
#   Contrôle de 'edgeFissIds' pour les anciennes versions
    if ( 'edgeFissIds' in self.dicoParams ):
      self.dicoParams['edgeFiss'] = self.dicoParams['edgeFissIds']
    if isinstance(self.dicoParams['edgeFiss'][0],int):
      geompy.UnionIDs(fondFiss, self.dicoParams['edgeFiss'] )
    else:
      l_groups = geompy.GetGroups(shellFiss)
      l_aux = list()
      for group in l_groups:
        if ( group.GetName() in self.dicoParams['edgeFiss'] ):
          l_aux.append(group)
      geompy.UnionList(fondFiss, l_aux )
    geomPublish(initLog.debug, shellFiss, 'shellFiss' )
    geomPublishInFather(initLog.debug, shellFiss, fondFiss, 'fondFiss' )


    coordsNoeudsFissure = genereMeshCalculZoneDefaut(shellFiss, self.dicoParams['meshBrep'][0] ,self.dicoParams['meshBrep'][1], \
                                                     mailleur, self.numeroCas)

    centre = None
    return [shellFiss, centre, lgInfluence, coordsNoeudsFissure, fondFiss]

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep           = self.reptrav,
                                      nomFicSain       = self.nomCas,
                                      nomFicFissure    = self.nomProbleme + "_fissure",
                                      nbsegRad         = self.dicoParams['nbSegRad'],
                                      nbsegCercle      = self.dicoParams['nbSegCercle'],
                                      areteFaceFissure = self.dicoParams['areteFaceFissure'],
                                      aretesVives      = self.dicoParams['aretesVives'])

  # ---------------------------------------------------------------------------
  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams):
    elementsDefaut = creeZoneDefautDansObjetSain(geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams, \
                          self.numeroCas)
    return elementsDefaut

  # ---------------------------------------------------------------------------
  def genereMaillageFissure(self, geometriesSaines, maillagesSains, \
                                  shapesFissure, shapeFissureParams, \
                                  maillageFissureParams, elementsDefaut, step, \
                                  mailleur="MeshGems"):
    maillageFissure = construitFissureGenerale(shapesFissure, shapeFissureParams, \
                                               maillageFissureParams, elementsDefaut, \
                                               mailleur, self.numeroCas)
    return maillageFissure

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    if self.references is not None:
      self.referencesMaillageFissure = self.references
    else:
      self.referencesMaillageFissure = dict( \
                                            Entity_Quad_Quadrangle = 0, \
                                            Entity_Quad_Hexa = 0, \
                                            Entity_Node = 0, \
                                            Entity_Quad_Edge = 0, \
                                            Entity_Quad_Triangle = 0, \
                                            Entity_Quad_Tetra = 0, \
                                            Entity_Quad_Pyramid = 0, \
                                            Entity_Quad_Penta = 0 \
                                           )
