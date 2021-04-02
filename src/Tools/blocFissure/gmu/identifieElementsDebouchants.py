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
"""Eléments débouchants (intersection pipe et peau), indexés selon les edges du fond de fissure (edgesFondIn)"""

import logging

from .identifieElementsDebouchants_a import identifieElementsDebouchants_a
from .identifieElementsDebouchants_b import identifieElementsDebouchants_b
from .identifieElementsDebouchants_c import identifieElementsDebouchants_c
from .identifieElementsDebouchants_d import identifieElementsDebouchants_d

def identifieElementsDebouchants(ifil, facesDefaut, partitionPeauFissFond, \
                                edgesFondIn, edgesFondFiss, wireFondFiss, \
                                aretesVivesC, fillingFaceExterne, \
                                edgesPipeIn, verticesPipePeau, rayonPipe, \
                                facesInside, facesOnside):
  """Eléments débouchants (intersection pipe et peau), indexés selon les edges du fond de fissure (edgesFondIn)"""

  logging.info('start')

  verticesEdgesFondIn = list() # les points du fond de fissure au débouché du pipe sur la peau (indice de edgesFondIn)
  pipexts = list()             # les segments de pipe associés au points de fond de fissure débouchants (même indice)
  cercles = list()             # les cercles de generation des pipes débouchant (même indice)
  facesFissExt = list()        # les faces de la fissure externe associés au points de fond de fissure débouchants (même indice)
  edgesFissExtPeau = list()    # edges des faces de fissure externe sur la peau (même indice)
  edgesFissExtPipe = list()    # edges des faces de fissure externe sur le pipe (même indice)

  #logging.debug("edgesFondIn %s", edgesFondIn)
  for iedf, edge in enumerate(edgesFondIn):

    ptPeau, centre, norm, localEdgeInFondFiss, localEdgeInFondFiss, cercle = \
                                   identifieElementsDebouchants_a(iedf, \
                                                                  partitionPeauFissFond, edgesFondFiss, wireFondFiss, \
                                                                  verticesPipePeau, rayonPipe, edge,)
    verticesEdgesFondIn.append(centre)
    cercles.append(cercle)

    # --- estimation de la longueur du pipe necessaire de part et d'autre du point de sortie
    loc_pt0, lgp = identifieElementsDebouchants_b(ifil, \
                                  facesDefaut,aretesVivesC, fillingFaceExterne, rayonPipe, \
                                  ptPeau, centre, norm, localEdgeInFondFiss)

    # --- position des points extremite du pipe sur l'edge debouchante
    identifieElementsDebouchants_c(iedf,\
                                   partitionPeauFissFond, wireFondFiss, \
                                   centre, localEdgeInFondFiss, cercle, loc_pt0, lgp, pipexts)

    # --- Recherche edges communes entre une face inside et (faces onside, edges pipe et fond débouchante)
    identifieElementsDebouchants_d(iedf,\
                                   partitionPeauFissFond, edgesFondIn, edgesPipeIn, \
                                   facesInside, facesOnside, \
                                   ptPeau, facesFissExt, edgesFissExtPeau, edgesFissExtPipe)

  return (verticesEdgesFondIn, pipexts, cercles, facesFissExt, edgesFissExtPeau, edgesFissExtPipe)
