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

def identifieFacesEdgesFissureExterne(fsFissuExt, edFisExtPe, edFisExtPi, edgesPipeFiss):
  """
  identification des faces et edges de fissure externe pour maillage
  """
  logging.info('start')
 
  facesFissExt = []
  edgesFissExtPeau = []
  edgesFissExtPipe = []
  for ifil in range(len(fsFissuExt)): # TODO: éliminer les doublons (comparer tous les vertices triés, avec mesure de distance ?)
    facesFissExt += fsFissuExt[ifil]
    edgesFissExtPeau += edFisExtPe[ifil]
    edgesFissExtPipe += edFisExtPi[ifil]
  logging.debug("---------------------------- identification faces de fissure externes au pipe :%s ", len(facesFissExt))
  # regroupement des faces de fissure externes au pipe.
  
  if len(facesFissExt) > 1:
    faceFissureExterne = geompy.MakePartition(facesFissExt, [], [], [], geompy.ShapeType["FACE"], 0, [], 0)
    edgesPipeFissureExterneC = geompy.GetInPlace(faceFissureExterne, geompy.MakeCompound(edgesPipeFiss))    # edgesFissExtPipe peut ne pas couvrir toute la longueur
    # edgesPeauFissureExterneC = geompy.GetInPlace(faceFissureExterne, geompy.MakeCompound(edgesFissExtPeau))
    # il peut manquer des edges de faceFissureExterne en contact avec la peau dans edgesFissExtPeau
    (isDone, closedFreeBoundaries, openFreeBoundaries) = geompy.GetFreeBoundary(faceFissureExterne)
    edgesBordFFE = []
    for bound in closedFreeBoundaries:
      edgesBordFFE += geompy.ExtractShapes(bound, geompy.ShapeType["EDGE"], False)
    edgesBordFFEid = [ (ed,geompy.GetSubShapeID(faceFissureExterne, ed)) for ed in edgesBordFFE]
    logging.debug("edgesBordFFEid %s", edgesBordFFEid)
    edgesPPE = geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False)
    edgesPPEid = [ geompy.GetSubShapeID(faceFissureExterne, ed) for ed in edgesPPE]
    logging.debug("edgesPPEid %s", edgesPPEid)
    edgesPFE = [ edid[0] for edid in edgesBordFFEid if edid[1] not in edgesPPEid] # on garde toutes les edges de bord non en contact avec le pipe
    logging.debug("edgesPFE %s", edgesPFE)
    edgesPeauFissureExterneC = geompy.MakeCompound(edgesPFE)
  else:
    faceFissureExterne = facesFissExt[0]
    edgesPeauFissureExterneC = geompy.MakeCompound(edgesFissExtPeau)
    edgesPipeFissureExterneC = geompy.MakeCompound(edgesFissExtPipe)
  wirePipeFissureExterne = geompy.MakeWire(geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False))
  geomPublish(initLog.debug, faceFissureExterne, "faceFissureExterne")
  geomPublishInFather(initLog.debug, faceFissureExterne, edgesPeauFissureExterneC, "edgesPeauFissureExterne")
  geomPublishInFather(initLog.debug, faceFissureExterne, edgesPipeFissureExterneC, "edgesPipeFissureExterne")
  
  return (faceFissureExterne, edgesPipeFissureExterneC, wirePipeFissureExterne, edgesPeauFissureExterneC)