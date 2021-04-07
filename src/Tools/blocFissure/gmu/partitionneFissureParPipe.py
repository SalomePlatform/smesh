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
"""Pipe de fond de fissure, prolongé, partition face fissure par pipe"""

import logging
import math
import traceback

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

from . import initLog
from .findWireEndVertices import findWireEndVertices
from .prolongeWire import prolongeWire
from .fissError import fissError

def partitionneFissureParPipe(shapesFissure, elementsDefaut, rayonPipe, \
                              nro_cas=None):
  """
  pipe de fond de fissure, prolongé, partition face fissure par pipe
  identification des edges communes pipe et face fissure
  """
  logging.info('start')
  shapeDefaut       = shapesFissure[0] # faces de fissure, débordant
  fondFiss          = shapesFissure[4] # groupe d'edges de fond de fissure
  centreFondFiss    = elementsDefaut[15]
  tgtCentre         = elementsDefaut[16]

  if geompy.NumberOfFaces(shapeDefaut) == 1:
    plan = geompy.MakePlane(centreFondFiss, tgtCentre, 10000)
    shapeDefaut = geompy.MakePartition([shapeDefaut], [plan], [], [], geompy.ShapeType["FACE"], 0, [], 0)
    #fondFissCoupe = geompy.GetInPlaceByHistory(shapeDefaut, fondFiss) #= inutile
    geomPublish(initLog.debug, shapeDefaut, 'shapeDefaut_coupe', nro_cas)
    #geomPublishInFather(initLog.debug,shapeDefaut, fondFissCoupe, 'fondFiss_coupe', nro_cas)

  extrem, norms = findWireEndVertices(fondFiss, True)
  logging.debug("extrem: %s, norm: %s",extrem, norms)
  cercle = geompy.MakeCircle(extrem[0], norms[0], rayonPipe)
  cercle = geompy.MakeRotation(cercle, norms[0], math.pi/3.0 ) # éviter d'avoir l'arête de couture du pipe presque confondue avec la face fissure
  geomPublish(initLog.debug, cercle, 'cercle', nro_cas)
  fondFissProlonge = prolongeWire(fondFiss, extrem, norms, 2*rayonPipe)
  try:
    pipeFiss = geompy.MakePipe(cercle, fondFissProlonge)
  except:
    texte = "génération du pipe le long de la ligne de fond de fissure prolongée impossible. "
    texte += "Cause possible : la ligne s'autointersecte lorsqu'on la prolonge."
    raise fissError(traceback.extract_stack(),texte)
  geomPublish(initLog.debug, pipeFiss, 'pipeFiss', nro_cas)
  partFissPipe = geompy.MakePartition([shapeDefaut, pipeFiss], [], [], [], geompy.ShapeType["FACE"], 0, [], 1)
  geomPublish(initLog.debug, partFissPipe, 'partFissPipe', nro_cas)
  fissPipe = geompy.GetInPlaceByHistory(partFissPipe, shapeDefaut)
  geomPublish(initLog.debug, fissPipe, 'fissPipe', nro_cas)
  partPipe = geompy.GetInPlaceByHistory(partFissPipe, pipeFiss)
  geomPublish(initLog.debug, partPipe, 'partPipe', nro_cas)

  edgesPipeFiss = geompy.GetSharedShapesMulti([fissPipe, partPipe], geompy.ShapeType["EDGE"])
  for i_aux, edge in enumerate(edgesPipeFiss):
    name = "edgePipe{}".format(i_aux)
    geomPublishInFather(initLog.debug,fissPipe, edge, name, nro_cas)
  try:
    wirePipeFiss = geompy.MakeWire(edgesPipeFiss)
  except:
    wirePipeFiss = geompy.MakeCompound(edgesPipeFiss)
    logging.debug("wirePipeFiss construit sous forme de compound")
  geomPublish(initLog.always, wirePipeFiss, "wirePipeFiss", nro_cas)

  wireFondFiss = geompy.GetInPlace(partFissPipe,fondFiss)
  edgesFondFiss = geompy.GetSharedShapesMulti([fissPipe, wireFondFiss], geompy.ShapeType["EDGE"])
  for i_aux, edge in enumerate(edgesFondFiss):
    name = "edgeFondFiss{}".format(i_aux)
    geomPublishInFather(initLog.debug,fissPipe, edge, name)
  wireFondFiss = geompy.MakeWire(edgesFondFiss)
  geomPublish(initLog.always, wireFondFiss, "wireFondFiss", nro_cas)

  return (fissPipe, edgesPipeFiss, edgesFondFiss, wirePipeFiss, wireFondFiss)
