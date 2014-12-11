# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

from sortFaces import sortFaces
from extractionOrientee import extractionOrientee

def identifieFacesPeau(ifil, verticesPipePeau, facesOnside, wireFondFiss,
                       verticesEdgesFondIn, pipexts, cercles,
                       fillingFaceExterne, centreFondFiss):
  """
  inventaire des faces de peau : face de peau percée du pipe, extrémités du pipe
  La partition avec le pipe peut créer un vertex (et un edge) de trop sur le cercle projeté,
  quand le cercle est très proche de la face.
  dans ce cas, la projection du cercle sur la face suivie d'une partition permet
  d'éviter le point en trop
  """
  logging.info('start')
   
  facesAndFond = facesOnside
  facesAndFond.append(wireFondFiss)
  try:
    partitionPeauByPipe = geompy.MakePartition(facesAndFond, pipexts, [], [], geompy.ShapeType["FACE"], 0, [], 1)
  except:
    logging.debug("probleme partition face pipe, contournement avec MakeSection")
    sections = []
    for pipext in pipexts:
      sections.append(geompy.MakeSection(facesOnside[0], pipext))
    partitionPeauByPipe = geompy.MakePartition(facesAndFond, sections, [], [], geompy.ShapeType["FACE"], 0, [], 1)
    
  # contrôle edge en trop sur edges circulaires
  if len(verticesPipePeau) > 0: # --- au moins une extrémité du pipe sur cette face de peau
    edgeEnTrop = []
    outilPart = pipexts
    facesPeau = geompy.ExtractShapes(partitionPeauByPipe, geompy.ShapeType["FACE"], False)
    facesPeauSorted, minsur, maxsurf = sortFaces(facesPeau)
    for i, face in enumerate(facesPeauSorted[:-1]): # on ne teste que la ou les petites faces "circulaires"
      nbv = geompy.NumberOfEdges(face)
      logging.debug("nombre d'edges sur face circulaire: %s", nbv)
      if nbv > 3:
        edgeEnTrop.append(True) # TODO : distinguer les cas avec deux faces circulaires dont l'une est correcte
      else:
        edgeEnTrop.append(False)
    refaire = sum(edgeEnTrop)
    if refaire > 0:
      dc = [(geompy.MinDistance(verticesEdgesFondIn[0], fac), i)  for i, fac in enumerate(facesPeauSorted[:-1])]
      dc.sort()
      logging.debug("dc sorted: %s", dc)
      i0 = dc[0][1] # indice de facesPeauSorted qui correspond à verticesEdgesFondIn[0], donc 0 pour cercles
      direct = (i0 == 0)
      for i, bad in enumerate(edgeEnTrop):
        if direct:
          j = i
        else:
          j = 1-i
        if bad:
          outilPart[j] = geompy.MakeProjection(cercles[j],facesOnside[0])
        pass
      partitionPeauByPipe = geompy.MakePartition(facesAndFond, outilPart, [], [], geompy.ShapeType["FACE"], 0, [], 1)
      pass
      
  name="partitionPeauByPipe%d"%ifil
  geomPublish(initLog.debug, partitionPeauByPipe, name)
  [edgesPeauFondIn, edgesPeauFondOut, edgesPeauFondOn] = extractionOrientee(fillingFaceExterne, partitionPeauByPipe, centreFondFiss, "EDGE", 1.e-3)
  [facesPeauFondIn, facesPeauFondOut, facesPeauFondOn] = extractionOrientee(fillingFaceExterne, partitionPeauByPipe, centreFondFiss, "FACE", 1.e-3)
    
  if len(verticesPipePeau) > 0: # --- au moins une extrémité du pipe sur cette face de peau
    facesPeauSorted, minsur, maxsurf = sortFaces(facesPeauFondOn)
    facePeau = facesPeauSorted[-1] # la plus grande face
  else:
    facePeau =geompy.MakePartition(facesPeauFondOn, [], [], [], geompy.ShapeType["FACE"], 0, [], 1)
    facesPeauSorted = [facePeau]
  name="facePeau%d"%ifil
  geomPublish(initLog.debug, facePeau, name)

  return (facePeau, facesPeauSorted, edgesPeauFondIn)