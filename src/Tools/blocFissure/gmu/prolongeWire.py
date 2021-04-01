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
"""Prolongation d'un wire par deux segments tangents"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublish

from . import initLog

from .orderEdgesFromWire import orderEdgesFromWire

def prolongeWire(aWire, extrem, norms, longueur):
  """Prolongation d'un wire par deux segments tangents"""
  logging.info("start")

  if ( geompy.NumberOfEdges(aWire) > 1 ):
    edges = geompy.ExtractShapes(aWire, geompy.ShapeType["EDGE"])
    uneSeuleEdge = False
  else:
    edges = [aWire]
    uneSeuleEdge = True

  edgesBout = list()
  for i_aux, v_1 in enumerate(extrem):
    exts = [geompy.MakeTranslationVectorDistance(v_1, norms[i_aux], lg_aux) for lg_aux in (-longueur, longueur)]
    dists = [(geompy.MinDistance(v, aWire), j_aux , v) for j_aux, v in enumerate(exts)]
    dists.sort()
    v_2 = dists[-1][-1]
    edge = geompy.MakeEdge(v_1, v_2)
    edges.append(edge)
    edgesBout.append(edge)
    name = "extrem{}".format(i_aux)
    geomPublish(initLog.debug, edge, name)

  try:
    wireProlonge = geompy.MakeWire(edges)
    geomPublish(initLog.debug, wireProlonge, "wireProlonge")
  except:
    logging.warning("probleme MakeWire, approche pas a pas")
    if uneSeuleEdge:
      edgelist = [aWire]
      accessList = [0]
    else:
      edgelist, accessList = orderEdgesFromWire(aWire)
    edge1 = edgelist[accessList[0]]
    if geompy.MinDistance(edgesBout[0], edge1) < 1.e-4 :
      i_0 = 0
      i_1 = 1
    else:
      i_0 = 1
      i_1 = 0
    wireProlonge = edgesBout[i_0]

    for i_aux in range(len(edgelist)):
      wireProlonge = geompy.MakeWire([wireProlonge, edgelist[accessList[i_aux]]])
      geomPublish(initLog.debug, wireProlonge, "wireProlonge_{}".format(i_aux))

    wireProlonge = geompy.MakeWire([wireProlonge,edgesBout[i_1]])
    geomPublish(initLog.debug, wireProlonge, "wireProlonge")
    logging.warning("prolongation wire pas a pas OK")

  return wireProlonge
