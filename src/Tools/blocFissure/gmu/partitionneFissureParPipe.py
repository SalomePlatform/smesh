# -*- coding: utf-8 -*-

import math
import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
from findWireEndVertices import findWireEndVertices
from prolongeWire import prolongeWire
import traceback
from fissError import fissError

def partitionneFissureParPipe(shapesFissure, elementsDefaut, rayonPipe):
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
    geomPublish(initLog.debug, shapeDefaut, 'shapeDefaut_coupe')
    #geomPublishInFather(initLog.debug,shapeDefaut, fondFissCoupe, 'fondFiss_coupe')
  
  extrem, norms = findWireEndVertices(fondFiss, True)
  logging.debug("extrem: %s, norm: %s",extrem, norms)
  cercle = geompy.MakeCircle(extrem[0], norms[0], rayonPipe)
  cercle = geompy.MakeRotation(cercle, norms[0], math.pi/3.0 ) # éviter d'avoir l'arête de couture du pipe presque confondue avec la face fissure
  geomPublish(initLog.debug, cercle, 'cercle')
  fondFissProlonge = prolongeWire(fondFiss, extrem, norms, 2*rayonPipe)
  try:
    pipeFiss = geompy.MakePipe(cercle, fondFissProlonge)
  except:
    texte = "génération du pipe le long de la ligne de fond de fissure prolongée impossible. "
    texte += "Cause possible : la ligne s'autointersecte lorsqu'on la prolonge."
    raise fissError(traceback.extract_stack(),texte)
  geomPublish(initLog.debug, pipeFiss, 'pipeFiss')
  partFissPipe = geompy.MakePartition([shapeDefaut, pipeFiss], [], [], [], geompy.ShapeType["FACE"], 0, [], 1)
  geomPublish(initLog.debug, partFissPipe, 'partFissPipe')
  fissPipe = geompy.GetInPlaceByHistory(partFissPipe, shapeDefaut)
  geomPublish(initLog.debug, fissPipe, 'fissPipe')
  partPipe = geompy.GetInPlaceByHistory(partFissPipe, pipeFiss)
  geomPublish(initLog.debug, partPipe, 'partPipe')
  
  edgesPipeFiss = geompy.GetSharedShapesMulti([fissPipe, partPipe], geompy.ShapeType["EDGE"])
  for i, edge in enumerate(edgesPipeFiss):
    name = "edgePipe%d"%i
    geomPublishInFather(initLog.debug,fissPipe, edge, name)
  try:
    wirePipeFiss = geompy.MakeWire(edgesPipeFiss)
  except:
    wirePipeFiss = geompy.MakeCompound(edgesPipeFiss)
    logging.debug("wirePipeFiss construit sous forme de compound")
  geomPublish(initLog.debug, wirePipeFiss, "wirePipeFiss")
  
  wireFondFiss = geompy.GetInPlace(partFissPipe,fondFiss)
  edgesFondFiss = geompy.GetSharedShapesMulti([fissPipe, wireFondFiss], geompy.ShapeType["EDGE"])
  for i, edge in enumerate(edgesFondFiss):
    name = "edgeFondFiss%d"%i
    geomPublishInFather(initLog.debug,fissPipe, edge, name)
  wireFondFiss = geompy.MakeWire(edgesFondFiss)
  geomPublish(initLog.debug, wireFondFiss,"wireFondFiss")  

  return (fissPipe, edgesPipeFiss, edgesFondFiss, wirePipeFiss, wireFondFiss)