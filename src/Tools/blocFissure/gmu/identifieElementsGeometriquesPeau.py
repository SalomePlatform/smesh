# -*- coding: utf-8 -*-

import logging

from identifieElementsFissure import identifieElementsFissure
from identifieElementsDebouchants import identifieElementsDebouchants
from trouveEdgesFissPeau import trouveEdgesFissPeau
from identifieFacesPeau import identifieFacesPeau
from identifieEdgesPeau import identifieEdgesPeau

def identifieElementsGeometriquesPeau(ifil, partitionPeauFissFond, edgesPipeFiss,
                                      edgesFondFiss, wireFondFiss, aretesVivesC,
                                      facesDefaut, centreFondFiss, rayonPipe,
                                      aretesVivesCoupees):
  """
  """
  logging.info('start')
  fillingFaceExterne = facesDefaut[ifil]

  logging.debug("traitement partitionPeauFissFond %s", ifil)
  
  # --- identification edges fond de fissure, edges pipe sur la face de fissure, edges prolongées
  #     edges internes communes pipe et fissure, points communs edges fissure peau et edges circulaires
  
  (edgesPipeIn, verticesPipePeau, edgesFondIn, facesInside, facesOnside) = identifieElementsFissure(ifil, facesDefaut, partitionPeauFissFond,
                                                                                                    edgesPipeFiss, edgesFondFiss, aretesVivesC,
                                                                                                    fillingFaceExterne, centreFondFiss)
  
  # --- elements débouchants (intersection pipe et peau), indexés selon les edges du fond de fissure (edgesFondIn)
  
  (verticesEdgesFondIn, pipexts, cercles,
   facesFissExt, edgesFissExtPeau, edgesFissExtPipe) = identifieElementsDebouchants(ifil, facesDefaut, partitionPeauFissFond,
                                                                                    edgesFondIn, edgesFondFiss, wireFondFiss,
                                                                                    aretesVivesC, fillingFaceExterne,
                                                                                    edgesPipeIn, verticesPipePeau, rayonPipe,
                                                                                    facesInside, facesOnside)

  # --- pour les faces de peau sans extremité débouchante de fissure, il faut recenser les edges de fissure sur la face de peau

  if len(verticesPipePeau) == 0: # aucune extrémité du pipe sur cette face de peau
    edgesFissExtPeau = trouveEdgesFissPeau(facesInside, facesOnside, edgesPipeIn, edgesFondIn, partitionPeauFissFond, edgesFissExtPeau)
    
  # --- inventaire des faces de peau : face de peau percée du pipe, extrémités du pipe
  
  (facePeau, facesPeauSorted, edgesPeauFondIn) = identifieFacesPeau(ifil, verticesPipePeau, facesOnside, wireFondFiss,
                                                                    verticesEdgesFondIn, pipexts, cercles,
                                                                    fillingFaceExterne, centreFondFiss)
  
  # --- identification précise des edges et disques des faces de peau selon index extremité fissure
  
  (endsEdgeFond, facesPipePeau, edgeRadFacePipePeau,
   edgesCircPeau, verticesCircPeau, groupEdgesBordPeau,
   bordsVifs, edgesFissurePeau, aretesVivesCoupees) = identifieEdgesPeau(edgesFissExtPipe, verticesPipePeau, facePeau, facesPeauSorted,
                                                                         edgesPeauFondIn, fillingFaceExterne, aretesVivesC, aretesVivesCoupees)
  
  dataPPFF = dict(endsEdgeFond        = endsEdgeFond,        # pour chaque face [points edge fond de fissure aux débouchés du pipe]
                  facesPipePeau       = facesPipePeau,       # pour chaque face [faces du pipe débouchantes]
                  edgeRadFacePipePeau = edgeRadFacePipePeau, # pour chaque face [edge radiale des faces du pipe débouchantes ]
                  facesFissExt        = facesFissExt,        # pour chaque face [faces de fissure externes au pipe]
                  edgesFissExtPeau    = edgesFissExtPeau,    # pour chaque face [edge en peau des faces de fissure externes (pas subshape facePeau)]
                  edgesFissExtPipe    = edgesFissExtPipe,    # pour chaque face [edge commun au pipe des faces de fissure externes]
                  facePeau            = facePeau,            # pour chaque face : la face de peau finale a mailler (percee des faces débouchantes)
                  edgesCircPeau       = edgesCircPeau,       # pour chaque face de peau : [groupe subshapes edges circulaires aux débouchés du pipe]
                  verticesCircPeau    = verticesCircPeau,    # pour chaque face de peau : [groupe subshapes points sur edges circulaires aux débouchés du pipe]
                  groupEdgesBordPeau  = groupEdgesBordPeau,  # pour chaque face de peau : groupe subshape des edges aux bords liés à la partie saine
                  bordsVifs           = bordsVifs,           # pour chaque face de peau : groupe subshape des edges aux bords correspondant à des arêtes vives
                  edgesFissurePeau    = edgesFissurePeau,    # pour chaque face de peau : [subshape edge en peau des faces de fissure externes]
                  verticesPipePeau    = verticesPipePeau     # pour chaque face de peau : [point commun edFissPeau edCircPeau]
                  )  

  return dataPPFF, aretesVivesCoupees
  
