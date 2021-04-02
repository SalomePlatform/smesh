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
"""Calcul de la position d'une shape par rapport à une face (dessus, dessous, sur la surface même)"""

import logging
from .geomsmesh import geompy

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
    else:
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
