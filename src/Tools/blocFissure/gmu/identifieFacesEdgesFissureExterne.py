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
"""Identification des faces et edges de fissure externe pour maillage"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

def identifieFacesEdgesFissureExterne(fsFissuExt, edFisExtPe, edFisExtPi, edgesPipeFiss, \
                                      nro_cas=None):
  """Identification des faces et edges de fissure externe pour maillage"""
  logging.info('start')

  texte = "---------------------------- fsFissuExt : {} ".format(fsFissuExt)
  logging.debug(texte)
  facesFissExt = list()
  edgesFissExtPeau = list()
  edgesFissExtPipe = list()
  for ifil, face in enumerate(fsFissuExt): # éliminer les doublons (comparer tous les vertices triés, avec mesure de distance ?)
    facesFissExt += face
    edgesFissExtPeau += edFisExtPe[ifil]
    edgesFissExtPipe += edFisExtPi[ifil]
  texte = "---------------------------- identification faces de fissure externes au pipe : {}".format(len(facesFissExt))
  logging.debug(texte)
  # regroupement des faces de fissure externes au pipe.

  if not facesFissExt:
    texte = "---------------------------- fsFissuExt : {} ".format(fsFissuExt)
    logging.info(texte)
    raise Exception("stop identifieFacesEdgesFissureExterne ; aucune face de fissure externe au pipe n'a été trouvée.")

  elif len(facesFissExt) > 1:
    faceFissureExterne = geompy.MakePartition(facesFissExt, [], [], [], geompy.ShapeType["FACE"], 0, [], 0)
    edgesPipeFissureExterneC = geompy.GetInPlace(faceFissureExterne, geompy.MakeCompound(edgesPipeFiss))    # edgesFissExtPipe peut ne pas couvrir toute la longueur
    # edgesPeauFissureExterneC = geompy.GetInPlace(faceFissureExterne, geompy.MakeCompound(edgesFissExtPeau))
    # il peut manquer des edges de faceFissureExterne en contact avec la peau dans edgesFissExtPeau
    (_, closedFreeBoundaries, _) = geompy.GetFreeBoundary(faceFissureExterne)
    edgesBordFFE = list()
    for bound in closedFreeBoundaries:
      edgesBordFFE += geompy.ExtractShapes(bound, geompy.ShapeType["EDGE"], False)
    edgesBordFFEid = [ (ed,geompy.GetSubShapeID(faceFissureExterne, ed)) for ed in edgesBordFFE]
    texte = "edgesBordFFEid {}".format(edgesBordFFEid)
    logging.debug(texte)
    edgesPPE = geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False)
    edgesPPEid = [ geompy.GetSubShapeID(faceFissureExterne, ed) for ed in edgesPPE]
    texte = "edgesPPEid {}".format(edgesPPEid)
    logging.debug(texte)
    edgesPFE = [ edid[0] for edid in edgesBordFFEid if edid[1] not in edgesPPEid] # on garde toutes les edges de bord non en contact avec le pipe
    texte = "edgesPFE {}".format(edgesPFE)
    logging.debug(texte)
    edgesPeauFissureExterneC = geompy.MakeCompound(edgesPFE)

  else:
    faceFissureExterne = facesFissExt[0]
    edgesPeauFissureExterneC = geompy.MakeCompound(edgesFissExtPeau)
    edgesPipeFissureExterneC = geompy.MakeCompound(edgesFissExtPipe)

  geomPublish(initLog.always, faceFissureExterne, "faceFissureExterne", nro_cas)
  geomPublishInFather(initLog.always, faceFissureExterne, edgesPeauFissureExterneC, "edgesPeauFissureExterne", nro_cas)
  geomPublishInFather(initLog.always, faceFissureExterne, edgesPipeFissureExterneC, "edgesPipeFissureExterne", nro_cas)

  wirePipeFissureExterne = geompy.MakeWire(geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False))

  return (faceFissureExterne, edgesPipeFissureExterneC, wirePipeFissureExterne, edgesPeauFissureExterneC)
