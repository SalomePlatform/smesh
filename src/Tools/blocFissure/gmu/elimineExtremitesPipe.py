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
"""Recherche des points en trop (externes au volume à remailler)"""

import logging

from .geomsmesh import geompy
from .whichSideVertex import whichSideVertex

def elimineExtremitesPipe(ptEdgeFond, facesDefaut, centres, gptsdisks, nbsegCercle):
  """Recherche des points en trop (externes au volume à remailler)

  - on associe chaque extrémité du pipe à une face filling
  - on part des disques aux extrémités du pipe
  - pour chaque disque, on prend les vertices de géométrie,
    on marque leur position relative à la face.
  - on s'arrete quand tous les noeuds sont dedans
  """

  logging.info('start')

  point_0 = centres[0]
  point_1 = centres[-1]
  idFillingFromBout = [None, None]                 # contiendra l'index du filling pour les extrémités 0 et 1
  nbFacesFilling = len(ptEdgeFond)
  for ifil in range(nbFacesFilling):
    for point in ptEdgeFond[ifil]: # il y a un ou deux points débouchant sur cette face
      if geompy.MinDistance(point,point_0) < geompy.MinDistance(point,point_1): # TODO: trouver plus fiable pour les cas tordus...
        idFillingFromBout[0] = ifil
      else:
        idFillingFromBout[1] = ifil
  logging.debug("association bouts du pipe - faces de filling: %s", idFillingFromBout)

  logging.debug("recherche des disques de noeuds complètement internes")
  idisklim = list() # indices des premier et dernier disques internes
  idiskout = list() # indices des premier et dernier disques externes
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
      idisk += inc
      logging.debug("examen disque %s", idisk)
      gptdsk = gptsdisks[idisk]
      inside = True
      for n_seg in range(nbsegCercle):
        points = gptdsk[n_seg]
        for point in points:
          side = whichSideVertex(facesDefaut[idFillingFromBout[bout]], point)
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
