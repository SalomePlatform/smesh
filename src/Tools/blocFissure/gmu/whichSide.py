# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- calcul de la position d'une shape par rapport à une face (dessus, dessous, sur la surface même)

def whichSide(face, obj, tol = 1.e-3):
  """
  Position d'un objet par rapport a une face (non nécessairement plane).
  L'objet est supposé situé entierement d'un coté de la face,
  ou lui appartenant totalement (objets traversants non pris en compte)
  renvoie 1 si 'objet est du coté de la normale à la face,
  -1 de l'autre coté, 0 si il est sur la face
  """
  logging.debug('start')
  side = 0
  logging.debug("shape info %s", geompy.ShapeInfo(obj))
  nbEdges = geompy.NbShapes(obj, geompy.ShapeType["EDGE"]) # --- attention ! pour une seule edge presente, renvoie 2
  logging.debug("   nbEdges %s", nbEdges)
  nbFaces = geompy.NbShapes(obj, geompy.ShapeType["FACE"]) # --- attention ! pour une seule face presente, renvoie 2
  logging.debug("   nbFaces %s", nbFaces)
  vertices = geompy.ExtractShapes(obj, geompy.ShapeType["VERTEX"], False)
  if nbEdges > 0 and nbFaces == 0: # --- edges
    if nbEdges <= 2:
      point = geompy.MakeVertexOnCurve(obj, 0.5)
      vertices.append(point)
    else:
      edges = geompy.ExtractShapes(obj, geompy.ShapeType["EDGE"], False)
      for anEdge in edges:
        point = geompy.MakeVertexOnCurve(anEdge, 0.5)
        vertices.append(point)
  elif nbFaces >0: # --- faces
    if nbFaces <=2:
      point = geompy.MakeVertexOnSurface(obj, 0.5, 0.5)
      vertices.append(point)
    if nbFaces > 2:
      faces = geompy.ExtractShapes(obj, geompy.ShapeType["FACE"], False)
      for aFace in faces:
        point = geompy.MakeVertexOnSurface(aFace, 0.5, 0.5)
        vertices.append(point)
  else: # --- vertices
    vertices = [obj]
  for vertex in vertices:
    distance = geompy.MinDistance(vertex, face)
    logging.debug("    distance %s", distance)
    if distance > tol:
      projection = geompy.MakeProjection(vertex, face)
      normal = geompy.GetNormal(face, projection)
      vect = geompy.MakeVector(projection, vertex)
      angle = geompy.GetAngle(normal, vect)
      logging.debug("  angle %s", angle)
      side = 1
      if abs(angle) > 10:
        side = -1
      break
  logging.debug("  side %s", side)
  return side

