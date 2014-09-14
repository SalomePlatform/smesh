# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from whichSideVertex import whichSideVertex

def elimineExtremitesPipe(ptEdgeFond, facesDefaut, centres, gptsdisks, nbsegCercle):
  """
  recherche des points en trop (externes au volume à remailler)
  - on associe chaque extrémité du pipe à une face filling 
  - on part des disques aux extrémités du pipe
  - pour chaque disque, on prend les vertices de géométrie,
    on marque leur position relative à la face.
  - on s'arrete quand tous les noeuds sont dedans
  """
  
  logging.info('start')

  pt0 = centres[0]
  pt1 = centres[-1]
  idFillingFromBout = [None, None]                 # contiendra l'index du filling pour les extrémités 0 et 1
  nbFacesFilling = len(ptEdgeFond)
  for ifil in range(nbFacesFilling):
    for ipt, pt in enumerate(ptEdgeFond[ifil]): # il y a un ou deux points débouchant sur cette face
      if geompy.MinDistance(pt,pt0) < geompy.MinDistance(pt,pt1): # TODO: trouver plus fiable pour les cas tordus...
        idFillingFromBout[0] = ifil
      else: 
        idFillingFromBout[1] = ifil
  logging.debug("association bouts du pipe - faces de filling: %s", idFillingFromBout)
     
  logging.debug("recherche des disques de noeuds complètement internes")
  idisklim = [] # indices des premier et dernier disques internes
  idiskout = [] # indices des premier et dernier disques externes
  for bout in range(2):
    if bout == 0:
      idisk = -1
      inc = 1
      numout = -1
    else:
      idisk = len(gptsdisks)
      inc = -1
      numout = len(gptsdisks)
    inside = False
    outside = True
    while not inside:
      idisk = idisk + inc
      logging.debug("examen disque %s", idisk)
      gptdsk = gptsdisks[idisk]
      inside = True
      for k in range(nbsegCercle):
        points = gptdsk[k]
        for j, pt in enumerate(points):
          side = whichSideVertex(facesDefaut[idFillingFromBout[bout]], pt)
          if side < 0:
            if outside: # premier point detecté dedans
              outside = False
              numout = idisk -inc # le disque précédent était dehors
          else:
            inside = False # ce point est dehors              
        if not inside and not outside:
          break
    idisklim.append(idisk)  # premier et dernier disques internes
    idiskout.append(numout) # premier et dernier disques externes
    
  return (idFillingFromBout, idisklim, idiskout)