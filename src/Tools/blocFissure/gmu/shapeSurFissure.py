# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

# -----------------------------------------------------------------------------
# --- construction d'une shape de dectection des éléments à modifier suite à la la duplication des noeuds de la face fissure (d'un coté de la face)

def shapeSurFissure(facesFissure):
  """
  TODO: a completer, Normaliser les vecteurs et ponderer par les surfaces...
  """
  logging.info('start')
  normal = None
  subIds = geompy.SubShapeAllIDs(facesFissure, geompy.ShapeType["FACE"])
  if len(subIds) > 1:
    logging.debug("plusieurs faces de fissure")
    faces = geompy.ExtractShapes(facesFissure, geompy.ShapeType["FACE"], False)
    extrusions = []
    for n,face in enumerate(faces):
      vertex = geompy.MakeVertexOnSurface(face, 0.5, 0.5)
      normal = geompy.GetNormal(face, vertex)
      extrusion = geompy.MakePrismVecH(face, normal, 100)
      extrusions.append(extrusion)
    extrusionFaceFissure = geompy.MakeCompound(extrusions)
  else:
    logging.debug("une seule face de fissure")
    face = facesFissure
    vertex = geompy.MakeVertexOnSurface(face, 0.5, 0.5)
    normal = geompy.GetNormal(face, vertex)
    extrusionFaceFissure = geompy.MakePrismVecH(facesFissure, normal, 100)
    
  geomPublish(initLog.debug, extrusionFaceFissure, "extrusionFaceFissure")
  return extrusionFaceFissure, normal

