# -*- coding: utf-8 -*-
# Copyright (C) 2014-2019  CEA/DEN, EDF R&D
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

# -----------------------------------------------------------------------------
# --- TORE
## --- faces 1/2 circulaires et leur centre, edges de ces faces dans le plan de fissure

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

  geomPublishInFather(initLog.debug, tore, faces[0], 'face0' )
  geomPublishInFather(initLog.debug, tore, faces[1], 'face1' )
  geomPublishInFather(initLog.debug, tore, faces[2], 'face2' )
  geomPublishInFather(initLog.debug, tore, faces[3], 'face3' )

  centres = [None, None, None, None]
  [v1,centres[0],v3] = geompy.ExtractShapes(faces[0], geompy.ShapeType["VERTEX"], True)
  [v1,centres[1],v3] = geompy.ExtractShapes(faces[1], geompy.ShapeType["VERTEX"], True)
  [v1,centres[2],v3] = geompy.ExtractShapes(faces[2], geompy.ShapeType["VERTEX"], True)
  [v1,centres[3],v3] = geompy.ExtractShapes(faces[3], geompy.ShapeType["VERTEX"], True)

  geomPublishInFather(initLog.debug, faces[0], centres[0], 'centre0' )
  geomPublishInFather(initLog.debug, faces[1], centres[1], 'centre1' )
  geomPublishInFather(initLog.debug, faces[2], centres[2], 'centre2' )
  geomPublishInFather(initLog.debug, faces[3], centres[3], 'centre3' )

  alledges = [None, None, None, None]
  alledges[0] = geompy.ExtractShapes(faces[0], geompy.ShapeType["EDGE"], True)
  alledges[1] = geompy.ExtractShapes(faces[1], geompy.ShapeType["EDGE"], True)
  alledges[2] = geompy.ExtractShapes(faces[2], geompy.ShapeType["EDGE"], True)
  alledges[3] = geompy.ExtractShapes(faces[3], geompy.ShapeType["EDGE"], True)

  dicoedge = {}
  edges = []
  reverses = []
  for i in range(len(alledges)):
    edgesface = alledges[i]
    lenef = []
    for j in range(len(edgesface)):
      props = geompy.BasicProperties(edgesface[j])
      lenef.append(props[0])
      pass
    maxlen = max(lenef)
    for j in range(len(edgesface)):
      if lenef[j] < maxlen:
        edgid = geompy.GetSubShapeID(tore, edgesface[j])
        if not (edgid in dicoedge):
          dicoedge[edgid] = edgesface[j]
          edges.append(edgesface[j])
          named = 'edge_' + str(i) + '_' +str(j)
          geomPublishInFather(initLog.debug, faces[i], edgesface[j], named)
          vertices = geompy.ExtractShapes(edgesface[j], geompy.ShapeType["VERTEX"], False)
          #firstVertex = geompy.GetFirstVertex(edgesface[j])
          if geompy.GetSubShapeID(tore, vertices[0]) != geompy.GetSubShapeID(tore, centres[i]):
            reverses.append(1)
            #print 'reversed ' + str(edgid)
          else:
            reverses.append(0)
            #print 'normal' + str(edgid)
          pass
        pass
      pass
    pass

  return faces, centres, edges, reverses

