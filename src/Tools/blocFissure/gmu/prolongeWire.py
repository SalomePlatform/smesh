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

import logging
from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog
from .orderEdgesFromWire import orderEdgesFromWire

# -----------------------------------------------------------------------------
# --- prolongation d'un wire par deux segments tangents

def prolongeWire(aWire, extrem, norms, lg):
  """
  """
  logging.info("start")
  if geompy.NumberOfEdges(aWire) > 1:
    edges = geompy.ExtractShapes(aWire, geompy.ShapeType["EDGE"])
    uneSeuleEdge = False
  else:
    edges = [aWire]
    uneSeuleEdge = True
  edgesBout = []
  for i, v1 in enumerate(extrem):
    exts = [geompy.MakeTranslationVectorDistance(v1, norms[i], l) for l in (-lg, lg)]
    dists = [(geompy.MinDistance(v, aWire), i , v) for i, v in enumerate(exts)]
    dists.sort()
    v2 = dists[-1][-1]
    edge = geompy.MakeEdge(v1, v2)
    edges.append(edge)
    edgesBout.append(edge)
    name = "extrem%d"%i
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
      i0 = 0
      i1 = 1
    else:
      i0 = 1
      i1 = 0
    wireProlonge = edgesBout[i0]
    for i in range(len(edgelist)):
      wireProlonge = geompy.MakeWire([wireProlonge, edgelist[accessList[i]]])
      geomPublish(initLog.debug, wireProlonge, "wireProlonge_%d"%i)
    wireProlonge = geompy.MakeWire([wireProlonge,edgesBout[i1]])
    geomPublish(initLog.debug, wireProlonge, "wireProlonge")
    logging.warning("prolongation wire pas a pas OK")
  return wireProlonge
