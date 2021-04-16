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
  logging.info("Traitement des arêtes de '%s'", facePeau.GetName())
  logging.info('Nombre de sommets : len(verticesPipePeau) = %d', len(verticesPipePeau))
  #logging.info('verticesPipePeau = %s', verticesPipePeau)
  logging.info('Nombre de sommets : len(verticesCircPeau) = %d', len(verticesCircPeau))
  #logging.info('verticesCircPeau = %s', verticesCircPeau)

  edgesPeau = geompy.ExtractShapes(facePeau, geompy.ShapeType["EDGE"], False)
  logging.info('Nombre total d arêtes de la peau : len(edgesPeau) = %d', len(edgesPeau))

  edges = substractSubShapes(facePeau, edgesPeau, edgesListees)

  edgesFissurePeau = list()
# --- au moins une extrémité du pipe sur cette face de peau
  if verticesPipePeau:
    # En premier, les edges associés aux extrémités du pipe
    edgesFissurePeau = [None for _ in range(len(verticesCircPeau))]
    for edge in edges:
      for i_aux, vertex in enumerate(verticesCircPeau):
        logging.info(".. distance %s", geompy.MinDistance(vertex, edge))
        if ( ( geompy.MinDistance(vertex, edge) < 1.e-3 ) and ( edge not in edgesFissurePeau ) ):
          edgesFissurePeau[i_aux] = edge
          name = "edgeFissurePeau_{}".format(i_aux)
          logging.info("... entrée de %s à la place %d", edge, i_aux)
          geomPublishInFather(initLog.debug, facePeau, edge, name)
    # Ensuite, on ajoute les edges manquantes
    for edge in edges:
      if edge not in edgesFissurePeau:
        logging.info("... ajout")
        edgesFissurePeau.append(edge)

  else:
    for i_aux, edge in enumerate(edges):
      edgesFissurePeau.append(edge)
      name = "edgeFissurePeau{}".format(i_aux)
      geomPublishInFather(initLog.debug, facePeau, edge, name)

  logging.info('==> Nombre d arêtes : len(edgesFissurePeau) = %d', len(edgesFissurePeau))

  return edgesFissurePeau
