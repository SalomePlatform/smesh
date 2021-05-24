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
"""edges de bord de la face de peau"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

def identifieEdgesPeau_b(facePeau, edgesListees, \
                         fillingFaceExterne, aretesVivesC, aretesVivesCoupees, \
                          nro_cas=None):
  """edges de bord de la face de peau"""
  logging.info('start')

  # Liste des arêtes de bord
  edgesFilling = geompy.ExtractShapes(fillingFaceExterne, geompy.ShapeType["EDGE"], False)
  logging.info('Détermination des arêtes de bord à partir des %d arêtes de fillingFaceExterne', len(edgesFilling))
  edgesBords = list()
  for i_aux, edge in enumerate(edgesFilling):
    edgepeau = geompy.GetInPlace(facePeau, edge)
    name = "edgepeau_{}".format(i_aux)
    geomPublishInFather(initLog.debug, facePeau,edgepeau, name)
    logging.debug("edgepeau %s", geompy.ShapeInfo(edgepeau))
    if geompy.ShapeInfo(edgepeau)['EDGE'] > 1:
      logging.debug("  EDGES multiples")
      l_edges = geompy.ExtractShapes(edgepeau, geompy.ShapeType["EDGE"], False)
      edgesBords.extend(l_edges)
      edgesListees.extend(l_edges)
    else:
      logging.debug("  EDGE")
      edgesBords.append(edgepeau)
      edgesListees.append(edgepeau)
  logging.info('==> Nombre d arêtes de bord : len(edgesBords) = %d', len(edgesBords))

  groupEdgesBordPeau = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
  geompy.UnionList(groupEdgesBordPeau, edgesBords)
  bordsVifs = None
  if aretesVivesC is not None:
    logging.info("identification des bords vifs par GetInPlace")
    bordsVifs = geompy.GetInPlace(facePeau, aretesVivesC)
    if bordsVifs is None:
      logging.debug("pas d'identification des bords vifs par GetInPlace: test par distance")
      edvifs = list()
      arvives = geompy.ExtractShapes(aretesVivesC, geompy.ShapeType["EDGE"], False)
      l_edges = geompy.ExtractShapes(facePeau, geompy.ShapeType["EDGE"], False)
      for edge in l_edges:
        vxs = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], False)
        for arete in arvives:
          dist = geompy.MinDistance(vxs[0], arete)
          dist += geompy.MinDistance(vxs[1], arete)
          logging.debug("test distance bord face peau - arete vive: %s",dist)
          if ( dist < 0.001 ):
            edvifs.append(edge)
            break
      if edvifs:
        logging.info('==> Nombre d arêtes de bord vif : len(edvifs) = %d', len(edvifs))
        bordsVifs = geompy.CreateGroup(facePeau,geompy.ShapeType["EDGE"])
        for edge in edvifs:
          geompy.AddObject(bordsVifs, geompy.GetSubShapeID(facePeau, edge))

  if bordsVifs is not None:
    geomPublishInFather(initLog.always, facePeau, bordsVifs, "bordsVifs")
    groupEdgesBordPeau = geompy.CutGroups(groupEdgesBordPeau, bordsVifs)
    grptmp = None
    if len(aretesVivesCoupees) > 0:
      grpc = geompy.MakeCompound(aretesVivesCoupees)
      grptmp = geompy.GetInPlace(facePeau, grpc)
    if grptmp is not None:
      grpnew = geompy.CutGroups(bordsVifs, grptmp) # ce qui est nouveau dans bordsVifs
    else:
      grpnew = bordsVifs
    if grpnew is not None:
      edv = geompy.ExtractShapes(grpnew, geompy.ShapeType["EDGE"], False)
      aretesVivesCoupees += edv

  logging.debug("aretesVivesCoupees %s",aretesVivesCoupees)
  geomPublishInFather(initLog.always, facePeau, groupEdgesBordPeau , "EdgesBords")

  return groupEdgesBordPeau, bordsVifs
