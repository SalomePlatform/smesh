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
"""Procédure complète de construction d'une fissure longue"""

import os

import logging

import salome

from .geomsmesh import geomPublish

from . import initLog

from .insereFissureLongue_a import insereFissureLongue_a
from .insereFissureLongue_b import insereFissureLongue_b
from .insereFissureLongue_c import insereFissureLongue_c
from .insereFissureLongue_d import insereFissureLongue_d
from .insereFissureLongue_e import insereFissureLongue_e
from .insereFissureLongue_f import insereFissureLongue_f
from .insereFissureLongue_g import insereFissureLongue_g

# -----------------------------------------------------------------------------

def insereFissureLongue(shapesFissure, shapeFissureParams, \
                        maillageFissureParams, elementsDefaut, \
                        mailleur="MeshGems", nro_cas=None):
  """Procédure complète de construction d'une fissure longue"""
  logging.info('start')
  logging.info("Usage du mailleur %s pour le cas n°%s", mailleur, nro_cas)

  shapeDefaut       = shapesFissure[0] # face de fissure, debordant
  #tailleDefaut      = shapesFissure[2]
  wiretube          = shapesFissure[4] # wire fond de fissure, debordant
  planfiss          = shapesFissure[7] # plan de fissure
  pipefiss          = shapesFissure[8] # pipe, debordant

  profondeur  = shapeFissureParams['profondeur']
  rayonPipe   = profondeur/4.0

  nomRep            = maillageFissureParams['nomRep']
  nomFicSain        = maillageFissureParams['nomFicSain']
  nomFicFissure     = maillageFissureParams['nomFicFissure']

  #nbsegExt          = maillageFissureParams['nbsegExt']      # 5
  #nbsegGen          = maillageFissureParams['nbsegGen']      # 25
  #nbsegRad          = maillageFissureParams['nbsegRad']      # 5
  #scaleRad          = maillageFissureParams['scaleRad']      # 4
  #nbsegCercle       = maillageFissureParams['nbsegCercle']   # 6
  #nbsegFis          = maillageFissureParams['nbsegFis']      # 20
  #lensegEllipsoide  = maillageFissureParams['lensegEllipso'] # 1.0

  #fichierMaillageSain = os.path.join(nomRep, '{}.med'.format(nomFicSain))
  fichierMaillageFissure = os.path.join(nomRep, '{}.med'.format(nomFicFissure))

  facesDefaut              = elementsDefaut[0]
  #centreDefaut             = elementsDefaut[1]
  #normalDefaut             = elementsDefaut[2]
  #extrusionDefaut          = elementsDefaut[3]
  #dmoyen                   = elementsDefaut[4]
  #bordsPartages            = elementsDefaut[5]
  #fillconts                = elementsDefaut[6]
  #idFilToCont              = elementsDefaut[7]
  maillageSain             = elementsDefaut[8]
  internalBoundary         = elementsDefaut[9]
  zoneDefaut               = elementsDefaut[10]
  zoneDefaut_skin          = elementsDefaut[11]
  zoneDefaut_internalFaces = elementsDefaut[12]
  zoneDefaut_internalEdges = elementsDefaut[13]

  facePorteFissure =  shapeDefaut
  WirePorteFondFissure = wiretube
  fillingFaceExterne = facesDefaut[0]
  logging.debug("fillingFaceExterne %s", fillingFaceExterne)
  geomPublish(initLog.debug, fillingFaceExterne, "fillingFaceExterne")

  # -----------------------------------------------------------------------------
  # --- peau et face de fissure
  edgesInside, centreFondFiss, tangentFondFiss, \
    planBord1, planBord2 , \
    facePeau, faceFiss, verticesOutCercles, verticesEdgePeauFiss, \
    edgePeauFiss, demiCerclesPeau, \
    groupEdgesBordPeau, groupsDemiCerclesPeau, groupEdgesFaceFissPipe = \
             insereFissureLongue_a (facePorteFissure, WirePorteFondFissure, \
                                    fillingFaceExterne, \
                                    pipefiss, rayonPipe, \
                                    mailleur, nro_cas )
  # -----------------------------------------------------------------------------
  # --- pipe de fond de fissure
  pipeFondFiss, disques, rayons, \
    demiCercles, generatrices, \
    VerticesEndPipeFiss, bordsLibres, \
    groupFaceFissInPipe, groupEdgeFondFiss, groupsDemiCerclesPipe, groupGenerFiss = \
            insereFissureLongue_b (edgesInside, centreFondFiss, tangentFondFiss, \
                                    planfiss, planBord1, planBord2, \
                                    facePeau, verticesOutCercles, verticesEdgePeauFiss, \
                                    fillingFaceExterne, rayonPipe, \
                                    internalBoundary, \
                                    nro_cas)

  # --- maillage pipe fond fissure

  meshFondFiss, groups_demiCercles, group_generFiss, nbSegGenLong, nbSegGenBout = \
            insereFissureLongue_c (pipeFondFiss, disques, rayons, demiCercles, demiCerclesPeau, generatrices, \
            VerticesEndPipeFiss, verticesEdgePeauFiss, \
            groupFaceFissInPipe, groupEdgeFondFiss, groupsDemiCerclesPipe, groupGenerFiss, \
            profondeur, rayonPipe, \
            nro_cas)

  # --- maillage face de peau

  meshFacePeau, groupEdgesPeauFiss = \
            insereFissureLongue_d ( facePeau, edgePeauFiss, groupEdgesBordPeau, bordsLibres, \
                                    groupsDemiCerclesPeau, groups_demiCercles, verticesOutCercles, \
                                    nbSegGenLong, nbSegGenBout, profondeur, \
                                    mailleur, nro_cas )

  # --- maillage face de fissure

  meshFaceFiss = \
            insereFissureLongue_e ( faceFiss, edgePeauFiss, groupEdgesPeauFiss, group_generFiss, groupEdgesFaceFissPipe, \
                                    profondeur, rayonPipe, \
                                    mailleur, nro_cas )

  # --- maillage meshBoiteDefaut

  meshBoiteDefaut, group_faceFissInPipe, group_faceFissOutPipe = \
            insereFissureLongue_f ( internalBoundary, meshFondFiss, meshFacePeau, meshFaceFiss, \
                                    mailleur, nro_cas )


  # --- maillage complet
  maillageComplet = \
            insereFissureLongue_g (nomFicFissure, fichierMaillageFissure, nomFicSain, maillageSain, \
                          meshBoiteDefaut, facePorteFissure, \
                          group_faceFissInPipe, group_faceFissOutPipe, \
                          zoneDefaut, zoneDefaut_skin, zoneDefaut_internalEdges, zoneDefaut_internalFaces, \
                          nro_cas)

  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser()

  return  maillageComplet
