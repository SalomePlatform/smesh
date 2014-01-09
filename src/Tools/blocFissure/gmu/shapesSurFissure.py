# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- identification des shapes modifiées par la duplication des noeuds de la face fissure (d'un coté de la face)

def shapesSurFissure(blocPartition, plane1, faceFissure, gencnt):
  """
  TODO: a completer
  """
  logging.info('start')

  shapesAModifier = []
  vertex = geompy.MakeVertexOnSurface(plane1, 0.5, 0.5)
  normal = geompy.GetNormal(plane1, vertex)
  extrusion = geompy.MakePrismVecH(plane1, normal, 100)

  sharedSolids = []
  solids= geompy.GetShapesOnBox ( extrusion, blocPartition, geompy.ShapeType("SOLID"), GEOM.ST_ONIN )
  for solid in solids:
    sharedSolids += geompy.GetSharedShapes(faceFissure, solid, geompy.ShapeType["SOLID"])
  logging.debug("sharedSolids %s",sharedSolids)

  sharedFaces = []
  faces= geompy.GetShapesOnBox ( extrusion, blocPartition, geompy.ShapeType("FACE"), GEOM.ST_ONIN )
  for face in faces:
    sharedFaces += geompy.GetSharedShapes(faceFissure, face, geompy.ShapeType["FACE"])
  logging.debug("sharedFaces %s",sharedFaces)

  sharedEdges = []
  edges= geompy.GetShapesOnBox ( extrusion, blocPartition, geompy.ShapeType("EDGE"), GEOM.ST_ONIN )
  for edge in edges:
    if not edge.IsSame(gencnt):
      sharedEdges += geompy.GetSharedShapes(faceFissure, edge, geompy.ShapeType["EDGE"])
  logging.debug("sharedEdges %s",sharedEdges)

  shapesAModifier = [ sharedSolids, sharedFaces, sharedEdges]
  return shapesAModifier
