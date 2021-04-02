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

def whichSideMulti(facerefs, ifil, obj, centre, tol = 1.e-3):
  """
  Position d'un objet par rapport a un ensemble de faces (non nécessairement planes).
  Ces faces délimitent partiellement un volume, le point "centre" est supposé dans le volume
  L'objet est supposé situé entierement d'un coté des faces,
  ou appartenant totalement à une face (objets traversants non pris en compte)
  renvoie 1 si 'objet est dedans (inside),
  -1 dehors (outside), 0 si il est sur une face d'index ifil
  """
  logging.debug('start')
  side = 0
  logging.debug("shape info %s", geompy.ShapeInfo(obj))
  nbEdges = geompy.NbShapes(obj, geompy.ShapeType["EDGE"]) # --- attention ! pour une seule edge presente, renvoie 2
  logging.debug("   nbEdges %s", nbEdges)
  vertices = geompy.ExtractShapes(obj, geompy.ShapeType["VERTEX"], False)

  if nbEdges > 0 : # --- edges
    if nbEdges <= 2:
      point = geompy.MakeVertexOnCurve(obj, 0.5)
      vertices.append(point)
    else:
      edges = geompy.ExtractShapes(obj, geompy.ShapeType["EDGE"], False)
      for anEdge in edges:
        point = geompy.MakeVertexOnCurve(anEdge, 0.5)
        vertices.append(point)
  else: # --- vertices
    vertices = [obj]

  nbOnRef = 0
  nbOnOther = 0
  for vertex in vertices:
    nbMiss = 0
    for i, face in enumerate(facerefs):
      distance = geompy.MinDistance(vertex, face)
      logging.debug("    distance %s", distance)
      if distance > tol:
        if geompy.MinDistance(vertex, centre) > tol:
          line = geompy.MakeLineTwoPnt(vertex, centre)
        else :
          side = 1       # le centre est inside...
          break
        part = geompy.MakePartition([line], [face], [], [], geompy.ShapeType["EDGE"], 0, [], 0)
        nbpts = geompy.NbShapes(part,geompy.ShapeType["VERTEX"])
        if nbpts > 2:
          side = -1      # outside
          break
        else:
          nbMiss += 1
          # peut-être inside, tester les autres faces
      else:
        if i == ifil:
          nbOnRef +=1    # le point est sur la face de référence, on continue avec les autres points
          break
        else:
          nbOnOther += 1 # le point est sur une autre face, mais il peut aussi être sur la face de référence...
         # on peut tester les autres faces
    if nbMiss == len(facerefs):
      side = 1 # inside
    if side != 0:
      break

  if side == 0 and nbOnRef < len(vertices):
    side = 1   # inside
  logging.debug("  side %s", side)

  return side
