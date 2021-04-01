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
"""Trouver les vertices intermédiaires d'un wire"""

import logging

from .geomsmesh import geompy

def orderEdgesFromWire(aWire):
  """Fournit les edges ordonnées d'un wire selon ExtractShapes(,,False),
  et l'ordre des edges selon le sens de parcours (ordre des indices de la liste d'edges)
  """
  logging.info("start")
  edges = geompy.ExtractShapes(aWire, geompy.ShapeType["EDGE"], False)

  idverts = dict()
  for i_aux, edge in enumerate(edges):
    verts = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], True)
    vertex_a = geompy.MakeVertexOnCurve(edge, 0.0)
    dist = geompy.MinDistance(vertex_a, verts[0])
    if dist < 1.e-4:
      idverts[(i_aux,0)] = verts[0]
      idverts[(i_aux,1)] = verts[1]
    else:
      idverts[(i_aux,0)] = verts[1]
      idverts[(i_aux,1)] = verts[0]

  idsubs = dict()
  for i_aux, sub in idverts.items():
    subid = geompy.GetSubShapeID(aWire, sub)
    if ( subid in idsubs ):
      idsubs[subid].append(i_aux)
    else:
      idsubs[subid] = [i_aux]

  debut = -1
  fin = -1
  for _, kvs in idsubs.items():
    if len(kvs) == 1: # une extremité
      kv0 = kvs[0]
      if kv0[1] == 0:
        debut = kv0[0]
      else:
        fin = kv0[0]
  texte = "nombre d'edges: {}, indice edge début: {}, fin: {}".format(len(edges), debut, fin)
  logging.debug(texte)
  if debut < 0:
    logging.critical("les edges du wire ne sont pas orientées dans le même sens: pas de début trouvé")
    return edges, list(range(len(edges)))

  orderedList = [debut]
  while len(orderedList) < len(edges):
    bout = orderedList[-1]
    vertex = idverts[(bout,1)]
    for k, vertex_a in idverts.items():
      if k[0] not in orderedList:
        if geompy.MinDistance(vertex, vertex_a) < 1.e-4:
          if k[1] == 0:
            orderedList.append(k[0])
            break
          else:
            logging.critical("les edges du wire ne sont pas orientées dans le même sens: une edge à l'envers")
            return edges, list(range(len(edges)))

  texte = "liste des edges ordonnées selon le sens de parcours : {}".format(orderedList)
  logging.debug(texte)
  accessList = list(range(len(orderedList)))
  for i_aux,k_aux in enumerate(orderedList):
    accessList[k_aux] = i_aux
  texte = "position ordonnée des edges selon le sens de parcours : {}".format(accessList)
  logging.info(texte)

  return edges, accessList
