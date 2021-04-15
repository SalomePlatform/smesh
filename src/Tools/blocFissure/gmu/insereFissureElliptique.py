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
"""Insertion d'une fissure elliptique"""

import os

import logging

import salome

from .partitionBlocDefaut import partitionBlocDefaut
from .facesVolumesToriques import facesVolumesToriques
from .facesCirculaires import facesCirculaires
from .propagateTore import propagateTore
from .sortGeneratrices import sortGeneratrices
from .facesFissure import facesFissure
from .facesToreInBloc import facesToreInBloc
from .shapeSurFissure import shapeSurFissure
from .meshBlocPart import meshBlocPart
from .enleveDefaut import enleveDefaut
from .regroupeSainEtDefaut import RegroupeSainEtDefaut
from .putName import putName

def insereFissureElliptique(geometriesSaines, maillagesSains, \
                            shapesFissure, shapeFissureParams, \
                            maillageFissureParams, elementsDefaut, step=-1):
  """procedure complete fissure elliptique"""
  logging.info('start')

  #geometrieSaine    = geometriesSaines[0]
  maillageSain      = maillagesSains[0]
  isHexa            = maillagesSains[1]
  #shapeDefaut       = shapesFissure[0]
  #tailleDefaut      = shapesFissure[2]
  #pipe0             = shapesFissure[4]
  gener1            = shapesFissure[5]
  pipe1             = shapesFissure[6]
  facefis1          = shapesFissure[7]
  plane1            = shapesFissure[8]
  ellipsoide1       = shapesFissure[9]


  #demiGrandAxe      = shapeFissureParams['demiGrandAxe']
  #demiPetitAxe      = shapeFissureParams['demiPetitAxe']
  #orientation       = shapeFissureParams['orientation']

  nomRep            = maillageFissureParams['nomRep']
  #nomFicSain        = maillageFissureParams['nomFicSain']
  nomFicFissure     = maillageFissureParams['nomFicFissure']

  nbsegExt          = maillageFissureParams['nbsegExt']      # 5
  nbsegGen          = maillageFissureParams['nbsegGen']      # 25
  nbsegRad          = maillageFissureParams['nbsegRad']      # 5
  scaleRad          = maillageFissureParams['scaleRad']      # 4
  nbsegCercle       = maillageFissureParams['nbsegCercle']   # 6
  nbsegFis          = maillageFissureParams['nbsegFis']      # 20
  lensegEllipsoide  = maillageFissureParams['lensegEllipso'] # 1.0

  #fichierMaillageSain = os.path.join(nomRep, '{}.med'.format(nomFicSain))
  fichierMaillageFissure = os.path.join(nomRep, '{}.med'.format(nomFicFissure))

  facesDefaut              = elementsDefaut[0]
  #centreDefaut             = elementsDefaut[1]
  #normalDefaut             = elementsDefaut[2]
  extrusionDefaut          = elementsDefaut[3]
  dmoyen                   = elementsDefaut[4]
  #bordsPartages            = elementsDefaut[5]
  #fillconts                = elementsDefaut[6]
  #idFilToCont              = elementsDefaut[7]
  #maillageSain             = elementsDefaut[8]
  internalBoundary         = elementsDefaut[9]
  zoneDefaut               = elementsDefaut[10]
  zoneDefaut_skin          = elementsDefaut[11]
  zoneDefaut_internalFaces = elementsDefaut[12]
  zoneDefaut_internalEdges = elementsDefaut[13]

  maillageComplet = None

  while True:

    ## --- ellipse incomplete : generatrice
    #if step == 5:
      #break

    #allonge = demiGrandAxe/demiPetitAxe
    #rayonTore = demiPetitAxe/5.0
    #generatrice, FaceGenFiss, Pipe_1, FaceFissure, Plane_1, Pipe1Part = self.toreFissure(demiPetitAxe, allonge, rayonTore)
    #ellipsoide = self.ellipsoideDefaut(demiPetitAxe, allonge)

    ## --- positionnement sur le bloc defaut de generatrice, tore et plan fissure
    #if step == 6:
      #break

    #pipe0 = self.rotTrans(Pipe_1, orientation, centreDefaut, normalDefaut)
    #gener1 = self.rotTrans(generatrice, orientation, centreDefaut, normalDefaut)
    #pipe1 = self.rotTrans(Pipe1Part, orientation, centreDefaut, normalDefaut)
    #facefis1 = self.rotTrans(FaceFissure, orientation, centreDefaut, normalDefaut)
    #plane1 = self.rotTrans(Plane_1, orientation, centreDefaut, normalDefaut)
    #ellipsoide1 = self.rotTrans(ellipsoide, orientation, centreDefaut, normalDefaut)

    #geomPublish(initLog.debug,  pipe0, 'pipe0' )
    #geomPublish(initLog.debug,  gener1, 'gener1' )
    #geomPublish(initLog.debug,  pipe1, 'pipe1' )
    #geomPublish(initLog.debug,  facefis1, 'facefis1' )
    #geomPublish(initLog.debug,  plane1, 'plane1' )
    #geomPublish(initLog.debug,  ellipsoide1, 'ellipsoide1' )

    # --- partition du bloc défaut par génératrice, tore et plan fissure
    if step == 7:
      break

    [ blocPartition, _, tore, \
      faceFissure, facesExternes, facesExtBloc, facesExtElli,
      aretesInternes, ellipsoidep, sharedFaces, sharedEdges, edgesBords] = \
      partitionBlocDefaut(extrusionDefaut, facesDefaut, gener1, pipe1, facefis1, ellipsoide1)
    if not isHexa:
      edgesBords = None # maillage sain hexa ==> filling, et maillage edges Bords imposés du maillage sain

    # --- TORE
    # --- faces toriques du tore
    if step == 8:
      break

    [facetore1, facetore2, _, _] = facesVolumesToriques(tore, plane1, facesDefaut)

    # --- faces 1/2 circulaires et edges dans le plan de fissure
    if step == 9:
      break

    [faces, centres, edges, reverses] = facesCirculaires(extrusionDefaut, tore)

    # --- recherche et classement des edges du tore par propagate
    if step == 10:
      break

    [diams, circles, geners] = propagateTore(tore)

    # --- tri par longueur des 3 génératrices
    if step == 11:
      break

    [_, genint, gencnt] = sortGeneratrices(tore, geners)

    # --- faces fissure dans et hors tore, et edges face hors tore
    if step == 12:
      break

    [_, facefissoutore, _, edgeext, reverext] = \
      facesFissure(ellipsoidep, faceFissure, extrusionDefaut, genint)

    # --- identification des faces tore et fissure dans le solide hors tore
    if step == 13:
      break

    [_, _, _] = \
      facesToreInBloc(ellipsoidep, facefissoutore, facetore1, facetore2)

    # --- identification des shapes modifiées par la duplication des noeuds de la face fissure (d'un coté de la face)

    #shapesAModifier = self.shapesSurFissure(blocPartition, plane1 faceFissure, gencnt)
    if step == 14:
      break

    extrusionFaceFissure, _ = shapeSurFissure(plane1)

    # --- maillage du bloc partitionne

    if step == 15:
      break

    [_, blocComplet] = \
      meshBlocPart(blocPartition, faceFissure, tore, centres, edges, diams, circles, faces, \
                  gencnt, facefissoutore, edgeext, facesExternes, facesExtBloc, facesExtElli, \
                  aretesInternes, internalBoundary, ellipsoidep, sharedFaces, sharedEdges, edgesBords, \
                  nbsegExt, nbsegGen, nbsegRad, scaleRad, reverses, reverext, nbsegCercle, \
                  nbsegFis, dmoyen, lensegEllipsoide)
    if step == 16:
      break

    maillageSain = enleveDefaut(maillageSain, zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges)
    if step == 17:
      break

    maillageComplet = RegroupeSainEtDefaut(maillageSain, blocComplet, extrusionFaceFissure, faceFissure, 'VOLUMES')
    if step == 18:
      break

    maillageComplet.ExportMED(fichierMaillageFissure)
    putName(maillageComplet, nomFicFissure)
    logging.info("fichier maillage fissure : %s", fichierMaillageFissure)

    break

  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser()

  return maillageComplet
