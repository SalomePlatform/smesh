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
"""procédure complète de construction d'une fissure générale"""

import logging

from . import initLog

import salome

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather
from .geomsmesh import smesh

from .orderEdgesFromWire import orderEdgesFromWire
from .restreintFaceFissure import restreintFaceFissure
from .partitionneFissureParPipe import partitionneFissureParPipe
from .construitPartitionsPeauFissure import construitPartitionsPeauFissure
from .compoundFromList import compoundFromList
from .identifieFacesEdgesFissureExterne import identifieFacesEdgesFissureExterne
from .calculePointsAxiauxPipe import calculePointsAxiauxPipe
from .elimineExtremitesPipe import elimineExtremitesPipe
from .construitEdgesRadialesDebouchantes import construitEdgesRadialesDebouchantes
from .creePointsPipePeau import creePointsPipePeau
from .ajustePointsEdgePipeFissure import ajustePointsEdgePipeFissure
from .construitMaillagePipe import construitMaillagePipe
from .mailleAretesEtJonction import mailleAretesEtJonction
from .mailleFacesFissure import mailleFacesFissure
from .mailleFacesPeau import mailleFacesPeau
from .putName import putName

from .construitFissureGenerale_a import construitFissureGenerale_a
from .construitFissureGenerale_b import construitFissureGenerale_b
from .construitFissureGenerale_c import construitFissureGenerale_c

# -----------------------------------------------------------------------------

def construitFissureGenerale(shapesFissure, shapeFissureParams, \
                             maillageFissureParams, elementsDefaut, \
                             mailleur="MeshGems", nro_cas=None):
  """procédure complète fissure générale"""
  logging.info('start')
  logging.info("Usage du mailleur %s pour le cas n°%s", mailleur, nro_cas)

  shapeDefaut       = shapesFissure[0] # faces de fissure, débordant

  rayonPipe = shapeFissureParams['rayonPipe']
  if 'lenSegPipe' in shapeFissureParams:
    lenSegPipe = shapeFissureParams['lenSegPipe']
  else:
    lenSegPipe = rayonPipe

  nbsegRad          = maillageFissureParams['nbsegRad']      # nombre de couches selon un rayon du pipe
  nbsegCercle       = maillageFissureParams['nbsegCercle']   # nombre de secteur dans un cercle du pipe
  areteFaceFissure  = maillageFissureParams['areteFaceFissure']
  lgAretesVives     = 0
  if 'aretesVives' in maillageFissureParams:
    lgAretesVives   = maillageFissureParams['aretesVives']

# Point interne
  pointInterne = construitFissureGenerale_a (geompy, shapeFissureParams)

  # fillings des faces en peau
  facesDefaut              = elementsDefaut[0]
  #centresDefaut            = elementsDefaut[1]
  #normalsDefaut            = elementsDefaut[2]
  #extrusionsDefaut         = elementsDefaut[3]
  dmoyen                   = elementsDefaut[4]
  bordsPartages            = elementsDefaut[5]
  #fillconts                = elementsDefaut[6]
  #idFilToCont              = elementsDefaut[7]
  maillageSain             = elementsDefaut[8]
  internalBoundary         = elementsDefaut[9]
  zoneDefaut               = elementsDefaut[10]
  zoneDefaut_skin          = elementsDefaut[11]
  zoneDefaut_internalFaces = elementsDefaut[12]
  zoneDefaut_internalEdges = elementsDefaut[13]
  #edgeFondExt              = elementsDefaut[14]
  centreFondFiss           = elementsDefaut[15]
  #tgtCentre                = elementsDefaut[16]
  if lgAretesVives == 0:
    lgAretesVives = dmoyen

  # --- restriction de la face de fissure au domaine solide :
  #     partition face fissure étendue par fillings, on garde la face interne

  facesPortFissure = restreintFaceFissure(shapeDefaut, facesDefaut, pointInterne, \
                                          nro_cas)

  # --- pipe de fond de fissure, prolongé, partition face fissure par pipe
  #     identification des edges communes pipe et face fissure

  (fissPipe, edgesPipeFiss, edgesFondFiss, wirePipeFiss, wireFondFiss) = \
                          partitionneFissureParPipe(shapesFissure, elementsDefaut, rayonPipe, \
                                                    nro_cas)
  edgesFondFiss, edgesIdByOrientation = orderEdgesFromWire(wireFondFiss)

  for i_aux, edge in enumerate(edgesFondFiss):
    geomPublishInFather(initLog.debug, wireFondFiss, edge, "edgeFondFiss{}".format(i_aux), nro_cas)

  # --- peau et face de fissure
  #
  # --- partition peau défaut - face de fissure prolongée - wire de fond de fissure prolongée
  #     il peut y avoir plusieurs faces externes, dont certaines sont découpées par la fissure
  #     liste de faces externes : facesDefaut
  #     liste de partitions face externe - fissure : partitionPeauFissFond (None quand pas d'intersection)

  partitionsPeauFissFond = construitPartitionsPeauFissure(facesDefaut, fissPipe)

  # --- arêtes vives détectées (dans quadranglesToShapeNoCorner
  #                             et quadranglesToShapeWithCorner)

  aretesVivesC = compoundFromList(bordsPartages, "areteVive")
  aretesVivesCoupees = list()  # ensemble des arêtes vives identifiées sur les faces de peau dans l'itération sur partitionsPeauFissFond

  # --- inventaire des faces de peau coupées par la fissure

  ptEdgeFond, fsFissuExt, edFisExtPe, edFisExtPi, facesPeaux, edCircPeau, \
    gpedgeBord, gpedgeVifs, edFissPeau, ptFisExtPi, \
    edgeRadFacePipePeau, facesPipePeau = \
    construitFissureGenerale_b( partitionsPeauFissFond, \
                                edgesPipeFiss, edgesFondFiss, wireFondFiss, aretesVivesC, \
                                facesDefaut, centreFondFiss, rayonPipe, aretesVivesCoupees, \
                                nro_cas )

  # --- identification des faces et edges de fissure externe pour maillage

  (faceFissureExterne, edgesPipeFissureExterneC, wirePipeFissureExterne, edgesPeauFissureExterneC) = \
      identifieFacesEdgesFissureExterne(fsFissuExt, edFisExtPe, edFisExtPi, edgesPipeFiss, \
                                        nro_cas)

  # --- préparation maillage du pipe :
  #     - détections des points a respecter : jonction des edges/faces constituant la face de fissure externe au pipe
  #     - points sur les edges de fond de fissure et edges pipe/face fissure,
  #     - vecteurs tangents au fond de fissure (normal au disque maillé)

  (centres, gptsdisks, raydisks) = calculePointsAxiauxPipe (edgesFondFiss, edgesIdByOrientation, facesDefaut, \
                                                            centreFondFiss, wireFondFiss, wirePipeFiss, \
                                                            lenSegPipe, rayonPipe, nbsegCercle, nbsegRad, \
                                                            nro_cas)

  # --- recherche des points en trop (externes au volume à remailler)
  #     - on associe chaque extrémité du pipe à une face filling
  #     - on part des disques aux extrémités du pipe
  #     - pour chaque disque, on prend les vertices de géométrie on marque leur position relative à la face.
  #     - on s'arrete quand tous les noeuds sont dedans

  (idFillingFromBout, idisklim, idiskout) = elimineExtremitesPipe(ptEdgeFond, facesDefaut, centres, gptsdisks, nbsegCercle)

  # --- construction des listes d'edges radiales sur chaque extrémité débouchante

  (listEdges, idFacesDebouchantes) = construitEdgesRadialesDebouchantes(idisklim, idiskout, gptsdisks, raydisks, \
                                                                        facesPipePeau, edgeRadFacePipePeau, nbsegCercle)

  # --- création des points du maillage du pipe sur la face de peau

  gptsdisks = creePointsPipePeau(listEdges, idFacesDebouchantes, idFillingFromBout, \
                                 ptEdgeFond, ptFisExtPi, edCircPeau, gptsdisks, idisklim, nbsegRad)

  # --- ajustement precis des points sur edgesPipeFissureExterneC

  gptsdisks = ajustePointsEdgePipeFissure(edgesPipeFissureExterneC, wirePipeFissureExterne, gptsdisks, idisklim)

   # --- maillage effectif du pipe

  (meshPipe, edgeFaceFissGroup, edgesCircPipeGroup) =  \
      construitMaillagePipe(gptsdisks, idisklim, nbsegCercle, nbsegRad, \
                             nro_cas)

  # --- edges de bord, faces défaut à respecter

  (internalBoundary, bordsLibres, grpAretesVives) =  \
      mailleAretesEtJonction(internalBoundary, aretesVivesCoupees, lgAretesVives, \
                             nro_cas)

  # --- maillage faces de fissure

  (meshFaceFiss, _, grpEdgesPeauFissureExterne, _) = \
      mailleFacesFissure(faceFissureExterne, \
                         edgesPipeFissureExterneC, edgesPeauFissureExterneC, \
                         edgeFaceFissGroup, areteFaceFissure, rayonPipe, nbsegRad, \
                         mailleur, nro_cas)

  # --- maillage faces de peau

  meshesFacesPeau = mailleFacesPeau(partitionsPeauFissFond, idFillingFromBout, facesDefaut, \
                                    facesPeaux, edCircPeau, gpedgeBord, gpedgeVifs, edFissPeau, \
                                    bordsLibres, grpEdgesPeauFissureExterne, grpAretesVives, \
                                    edgesCircPipeGroup, dmoyen, rayonPipe, nbsegRad, \
                                    mailleur, nro_cas)

  # --- regroupement des maillages du défaut

  listMeshes = [internalBoundary.GetMesh(), meshPipe.GetMesh(), meshFaceFiss.GetMesh()]
  for mfpeau in meshesFacesPeau:
    listMeshes.append(mfpeau.GetMesh())

  # Attention à la précision... 1.e-5 est trop exigeant. Il faudrait mettre une valeur en cohérence avec les tailles de mailles.
  meshBoiteDefaut = smesh.Concatenate(listMeshes, 1, 1, 1.e-04)
  putName(meshBoiteDefaut, "boiteDefaut", i_pref=nro_cas)

# Maillage complet
  maillageComplet = construitFissureGenerale_c( maillageSain, meshBoiteDefaut, \
                                                zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges, \
                                                facesPortFissure, \
                                                maillageFissureParams, \
                                                mailleur, nro_cas )

  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser()

  logging.info("maillage fissure fini")

  return maillageComplet
