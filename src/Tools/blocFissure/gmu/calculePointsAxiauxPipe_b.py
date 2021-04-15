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
"""Préparation maillage du pipe"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

from . import initLog

def calculePointsAxiauxPipe_b(meshFondFiss, \
                              edgesFondFiss, edgesIdByOrientation, \
                              wireFondFiss, wirePipeFiss, \
                              rayonPipe):
  """Préparation maillage du pipe :

  - détections des points a respecter : jonction des edges/faces constituant la face de fissure externe au pipe
  - points sur les edges de fond de fissure et edges pipe/face fissure,
  - vecteurs tangents au fond de fissure (normal au disque maillé)
  """

  logging.info('start')

  ptGSdic = dict() # dictionnaire [paramètre sur la courbe] --> point géométrique
  allNodeIds = meshFondFiss.GetNodesId()
  for nodeId in allNodeIds:
    xyz = meshFondFiss.GetNodeXYZ(nodeId)
    #logging.debug("nodeId %s, coords %s", nodeId, str(xyz))
    point = geompy.MakeVertex(xyz[0], xyz[1], xyz[2])
    parametre, _, EdgeInWireIndex = geompy.MakeProjectionOnWire(point, wireFondFiss) # parametre compris entre 0 et 1
    edgeOrder = edgesIdByOrientation[EdgeInWireIndex]
    ptGSdic[(edgeOrder, EdgeInWireIndex, parametre)] = point
    #logging.debug("nodeId %s, parametre %s", nodeId, str(parametre))
  usort = sorted(ptGSdic)
  logging.debug("nombre de points obtenus par deflexion : %s",len(usort))

  centres = list()
  origins = list()
  normals = list()
  for i_aux, edu in enumerate(usort):
    vertcx = ptGSdic[edu]
    geomPublishInFather(initLog.debug, wireFondFiss, vertcx, "vertcx_{}".format(i_aux))
    norm = geompy.MakeTangentOnCurve(edgesFondFiss[edu[1]], edu[2])
    plan = geompy.MakePlane(vertcx, norm, 3.*rayonPipe)
    part = geompy.MakePartition([plan], [wirePipeFiss], list(), list(), geompy.ShapeType["VERTEX"], 0, list(), 0)
    liste = geompy.ExtractShapes(part, geompy.ShapeType["VERTEX"], True)
    if ( len(liste) == 5 ): # 4 coins du plan plus intersection recherchée
      for point in liste:
        if geompy.MinDistance(point, vertcx) < 1.1*rayonPipe: # les quatre coins sont plus loin
          vertpx = point
          geomPublishInFather(initLog.debug, wireFondFiss, vertpx, "vertpx_{}".format(i_aux))
          break
      centres.append(vertcx)
      origins.append(vertpx)
      normals.append(norm)

  return centres, origins, normals
