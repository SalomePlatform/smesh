# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

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