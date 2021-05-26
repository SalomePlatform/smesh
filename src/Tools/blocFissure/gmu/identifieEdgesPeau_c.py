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

  edgesPeau = geompy.ExtractShapes(facePeau, geompy.ShapeType["EDGE"], False)
  logging.info('Nombre total d arêtes de la peau : len(edgesPeau) = %d', len(edgesPeau))

  edges = substractSubShapes(facePeau, edgesPeau, edgesListees)

  l_edge_cercle = list()
# --- au moins une extrémité du pipe sur cette face de peau : arêtes sui les prenent toutes
  if verticesPipePeau:
    i_aux = -1
    for edge in edges:
      for groupe in verticesCircPeau:
        cercle = True
        for id_vertex in geompy.GetObjectIDs(groupe):
          vertex = geompy.GetSubShape(facePeau, [id_vertex])
          distance = geompy.MinDistance(vertex, edge)
          logging.info(".. distance %s", distance)
          if ( distance > 1.e-3 ):
            cercle = False
            break
        if cercle:
          logging.info("... entrée de %s dans l_edge_cercle", edge)
          l_edge_cercle.append(edge)
          i_aux += 1
          name = "edgeFissurePeauCercle_{}".format(i_aux)
          geomPublishInFather(initLog.info, facePeau, edge, name)

# --- Les arêtes n'appartenant pas aux cercles
  edgesFissurePeau = list()
  for i_aux, edge in enumerate(edges):
    if edge not in l_edge_cercle:
      edgesFissurePeau.append(edge)
      name = "edgeFissurePeau_{}".format(i_aux)
      geomPublishInFather(initLog.debug, facePeau, edge, name)

  logging.info('==> Nombre d arêtes : len(edgesFissurePeau) = %d', len(edgesFissurePeau))

  return edgesFissurePeau
