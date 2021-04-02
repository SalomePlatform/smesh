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
"""edges de la face de peau partagées avec la face de fissure"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

from .substractSubShapes import substractSubShapes

def identifieEdgesPeau_c(verticesPipePeau, facePeau, edgesListees, verticesCircPeau):
  """edges de la face de peau partagées avec la face de fissure"""
  logging.info('start')

  edgesPeau = geompy.ExtractShapes(facePeau, geompy.ShapeType["EDGE"], False)
  edges = substractSubShapes(facePeau, edgesPeau, edgesListees)
  edgesFissurePeau = list()

  if len(verticesPipePeau) > 0: # --- au moins une extrémité du pipe sur cette face de peau
    edgesFissurePeau = [None for _ in range(len(verticesCircPeau))] # edges associés aux extrémités du pipe, en premier
    for edge in edges:
      for i_aux, grpVert in enumerate(verticesCircPeau):
        if (geompy.MinDistance(grpVert, edge) < 1.e-3) and (edge not in edgesFissurePeau):
          edgesFissurePeau[i_aux] = edge
          name = "edgeFissurePeau{}".format(i_aux)
          geomPublishInFather(initLog.debug, facePeau,  edge, name)
    for edge in edges: # on ajoute après les edges manquantes
      if edge not in edgesFissurePeau:
        edgesFissurePeau.append(edge)

  else:
    for i_aux, edge in enumerate(edges):
      edgesFissurePeau.append(edge)
      name = "edgeFissurePeau{}".format(i_aux)
      geomPublishInFather(initLog.debug, facePeau,  edge, name)

  return edgesFissurePeau
