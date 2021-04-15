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
"""Tore : faces 1/2 circulaires et leur centre, edges de ces faces dans le plan de fissure"""

import logging

from . import initLog

import GEOM

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

def facesCirculaires(bloc, tore):
  """
  Extraction des faces demi circulaires à l'intersection du tore partitionné et de la paroi,
  de leur centre, les edges de ces faces situees dans le plan de fissure et un booleen par edge,
  indiquant son sens (normal / reversed).
  @param bloc : bloc defaut
  @param tore : le tore partitionné et coupé
  @return (faces, centres, edges, reverses)
  """
  logging.info("start")

  faces = geompy.GetShapesOnShape(bloc, tore, geompy.ShapeType["FACE"], GEOM.ST_ON)

  centres = list()
  alledges = list()
  for i_aux in range(4):
    geomPublishInFather(initLog.debug, tore, faces[i_aux], 'face{}'.format(i_aux))
    [_,centre,_] = geompy.ExtractShapes(faces[i_aux], geompy.ShapeType["VERTEX"], True)
    centres.append(centre)
    geomPublishInFather(initLog.debug, faces[i_aux], centre, 'centre{}'.format(i_aux))
    alledges.append(geompy.ExtractShapes(faces[i_aux], geompy.ShapeType["EDGE"], True))

  dicoedge = dict()
  edges = list()
  reverses = list()

  for i_aux, edgesface in enumerate(alledges):

    lenef = list()
    for edge in edgesface:
      props = geompy.BasicProperties(edge)
      lenef.append(props[0])

    maxlen = max(lenef)
    for j_aux, edge in enumerate(edgesface):
      if lenef[j_aux] < maxlen:
        edgid = geompy.GetSubShapeID(tore, edge)
        if not (edgid in dicoedge):
          dicoedge[edgid] = edge
          edges.append(edge)
          named = 'edge_{}_{}'.format(i_aux,j_aux)
          geomPublishInFather(initLog.debug, faces[i_aux], edge, named)
          vertices = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], False)
          #firstVertex = geompy.GetFirstVertex(edge)
          if geompy.GetSubShapeID(tore, vertices[0]) != geompy.GetSubShapeID(tore, centres[i_aux]):
            reverses.append(1)
            #print 'reversed ' + str(edgid)
          else:
            reverses.append(0)
            #print 'normal' + str(edgid)

  return faces, centres, edges, reverses
