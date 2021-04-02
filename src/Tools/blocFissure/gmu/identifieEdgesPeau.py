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
"""Identification précise des edges et disques des faces de peau selon index extremité fissure"""

import logging

from .identifieEdgesPeau_a import identifieEdgesPeau_a
from .identifieEdgesPeau_b import identifieEdgesPeau_b
from .identifieEdgesPeau_c import identifieEdgesPeau_c

def identifieEdgesPeau(edgesFissExtPipe, verticesPipePeau, facePeau, facesPeauSorted, \
                       edgesPeauFondIn, fillingFaceExterne, aretesVivesC, aretesVivesCoupees):
  """Identification précise des edges et disques des faces de peau selon index extremité fissure"""
  logging.info('start')

  facesPipePeau = [None for _ in range(len(edgesFissExtPipe))]
  endsEdgeFond = [None for _ in range(len(edgesFissExtPipe))]
  edgeRadFacePipePeau = [None for _ in range(len(edgesFissExtPipe))]

  edgesListees = list()
  if len(verticesPipePeau) > 0: # --- au moins une extrémité du pipe sur cette face de peau

    edgesCircPeau, verticesCircPeau = identifieEdgesPeau_a(edgesFissExtPipe, facePeau, facesPeauSorted, edgesPeauFondIn, \
                                                           endsEdgeFond, facesPipePeau, edgeRadFacePipePeau, edgesListees)
  else:
    edgesCircPeau = list()
    verticesCircPeau = list()


  # --- edges de bord de la face de peau

  groupEdgesBordPeau, bordsVifs = identifieEdgesPeau_b(facePeau, edgesListees, \
                                                      fillingFaceExterne, aretesVivesC, aretesVivesCoupees)

  # ---  edges de la face de peau partagées avec la face de fissure

  edgesFissurePeau = identifieEdgesPeau_c(verticesPipePeau, facePeau, edgesListees, verticesCircPeau)

  return (endsEdgeFond, facesPipePeau, edgeRadFacePipePeau,
          edgesCircPeau, verticesCircPeau, groupEdgesBordPeau,
          bordsVifs, edgesFissurePeau, aretesVivesCoupees)
