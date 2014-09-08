# -*- coding: utf-8 -*-

import math
import logging
from geomsmesh import geompy
from findWireEndVertices import findWireEndVertices
from prolongeWire import prolongeWire

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
    geompy.addToStudy(shapeDefaut, 'shapeDefaut_coupe')
    #geompy.addToStudyInFather(shapeDefaut, fondFissCoupe, 'fondFiss_coupe')
  
  extrem, norms = findWireEndVertices(fondFiss, True)
  logging.debug("extrem: %s, norm: %s",extrem, norms)
  cercle = geompy.MakeCircle(extrem[0], norms[0], rayonPipe)
  cercle = geompy.MakeRotation(cercle, norms[0], math.pi/3.0 ) # éviter d'avoir l'arête de couture du pipe presque confondue avec la face fissure
  geompy.addToStudy(cercle, 'cercle')
  fondFissProlonge = prolongeWire(fondFiss, extrem, norms, 2*rayonPipe)
  pipeFiss = geompy.MakePipe(cercle, fondFissProlonge)
  geompy.addToStudy(pipeFiss, 'pipeFiss')
  partFissPipe = geompy.MakePartition([shapeDefaut, pipeFiss], [], [], [], geompy.ShapeType["FACE"], 0, [], 1)
  geompy.addToStudy(partFissPipe, 'partFissPipe')
  fissPipe = geompy.GetInPlaceByHistory(partFissPipe, shapeDefaut)
  geompy.addToStudy(fissPipe, 'fissPipe')
  partPipe = geompy.GetInPlaceByHistory(partFissPipe, pipeFiss)
  geompy.addToStudy(partPipe, 'partPipe')
  
  edgesPipeFiss = geompy.GetSharedShapesMulti([fissPipe, partPipe], geompy.ShapeType["EDGE"])
  for i, edge in enumerate(edgesPipeFiss):
    name = "edgePipe%d"%i
    geompy.addToStudyInFather(fissPipe, edge, name)
  try:
    wirePipeFiss = geompy.MakeWire(edgesPipeFiss)
  except:
    wirePipeFiss = geompy.MakeCompound(edgesPipeFiss)
    logging.debug("wirePipeFiss construit sous forme de compound")
  geompy.addToStudy(wirePipeFiss, "wirePipeFiss")
  
  wireFondFiss = geompy.GetInPlace(partFissPipe,fondFiss)
  edgesFondFiss = geompy.GetSharedShapesMulti([fissPipe, wireFondFiss], geompy.ShapeType["EDGE"])
  for i, edge in enumerate(edgesFondFiss):
    name = "edgeFondFiss%d"%i
    geompy.addToStudyInFather(fissPipe, edge, name)
  wireFondFiss = geompy.MakeWire(edgesFondFiss)
  geompy.addToStudy(wireFondFiss,"wireFondFiss")  

  return (fissPipe, edgesPipeFiss, edgesFondFiss, wirePipeFiss, wireFondFiss)