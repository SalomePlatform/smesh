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
"""Identification précise des edges et disques des faces de peau selon index extremité fissure"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

def identifieEdgesPeau_a(edgesFissExtPipe, facePeau, facesPeauSorted, edgesPeauFondIn, \
                         endsEdgeFond, facesPipePeau, edgeRadFacePipePeau, edgesListees):
  """Identification précise des edges et disques des faces de peau selon index extremité fissure"""
  logging.info('start')

  for face in facesPeauSorted[:-1]: # la ou les faces débouchantes, pas la grande face de peau
    logging.debug("examen face debouchante circulaire")
    for i_aux,efep in enumerate(edgesFissExtPipe):
      dist = geompy.MinDistance(face, efep)
      logging.debug("  distance face circulaire edge %s", dist)
      if dist < 1e-3:
        for j_aux, edpfi in enumerate(edgesPeauFondIn):
          if geompy.MinDistance(face, edpfi) < 1e-3:
            j_aux_0 = j_aux
            break
        sharedVertices = geompy.GetSharedShapesMulti([face, edgesPeauFondIn[j_aux_0]], geompy.ShapeType["VERTEX"])
        nameFace = "facePipePeau{}".format(i_aux)
        nameVert = "endEdgeFond{}".format(i_aux)
        nameEdge = "edgeRadFacePipePeau{}".format(i_aux)
        facesPipePeau[i_aux] = face
        endsEdgeFond[i_aux] = sharedVertices[0]
        geomPublish(initLog.debug, face, nameFace)
        geomPublish(initLog.debug, sharedVertices[0], nameVert)
        edgesFace = geompy.ExtractShapes(face, geompy.ShapeType["EDGE"], True)
        for edge in edgesFace:
          if geompy.MinDistance(edge, sharedVertices[0]) < 1e-3:
            edgeRadFacePipePeau[i_aux] = edge
            geomPublish(initLog.debug, edge, nameEdge)
            break

  # --- edges circulaires de la face de peau et points de jonction de la face externe de fissure
  logging.debug("facesPipePeau: %s", facesPipePeau)
  edgesCircPeau = [None for _ in range(len(facesPipePeau))]
  verticesCircPeau = [None for _ in range(len(facesPipePeau))]
  for i_aux,fcirc in enumerate(facesPipePeau):
    edges = geompy.GetSharedShapesMulti([facePeau, fcirc], geompy.ShapeType["EDGE"])
    grpEdgesCirc = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
    geompy.UnionList(grpEdgesCirc, edges)
    edgesCircPeau[i_aux] = grpEdgesCirc
    name = "edgeCirc{}".format(i_aux)
    geomPublishInFather(initLog.debug, facePeau, grpEdgesCirc, name)
    edgesListees = edgesListees + edges
    vertices = geompy.GetSharedShapesMulti([facePeau, fcirc], geompy.ShapeType["VERTEX"])
    grpVertCircPeau = geompy.CreateGroup(facePeau, geompy.ShapeType["VERTEX"])
    geompy.UnionList(grpVertCircPeau, vertices)
    verticesCircPeau[i_aux] = grpVertCircPeau
    name = "pointEdgeCirc{}".format(i_aux)
    geomPublishInFather(initLog.debug, facePeau, grpVertCircPeau, name)

  return edgesCircPeau, verticesCircPeau
