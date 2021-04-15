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
"""Construit les arêtes débouchantes"""

import logging

from .construitEdgesRadialesDebouchantes_a import construitEdgesRadialesDebouchantes_a
from .construitEdgesRadialesDebouchantes_b import construitEdgesRadialesDebouchantes_b
from .construitEdgesRadialesDebouchantes_c import construitEdgesRadialesDebouchantes_c

def construitEdgesRadialesDebouchantes(idisklim, idiskout, gptsdisks, raydisks,
                                       facesPipePeau, edgeRadFacePipePeau, nbsegCercle):
  """construction des listes d'edges radiales sur chaque extrémité débouchante"""
  logging.info('start')

  # --- listes de nappes radiales en filling à chaque extrémité débouchante

  facesDebouchantes, listNappes = construitEdgesRadialesDebouchantes_a(idisklim, idiskout, gptsdisks, raydisks, nbsegCercle)

  # --- mise en correspondance avec les indices des faces disque débouchantes (facesPipePeau)
  idFacesDebouchantes = construitEdgesRadialesDebouchantes_b(facesPipePeau, facesDebouchantes, listNappes)

  # --- construction des listes d'edges radiales sur chaque extrémité débouchante
  listEdges = construitEdgesRadialesDebouchantes_c(facesPipePeau, edgeRadFacePipePeau, \
                                                   listNappes, idFacesDebouchantes)

  return (listEdges, idFacesDebouchantes)
