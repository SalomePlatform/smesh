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
                         endsEdgeFond, facesPipePeau, edgeRadFacePipePeau, edgesListees, \
                        nro_cas=None):
  """Identification précise des edges et disques des faces de peau selon index extremité fissure"""
  logging.info('start')
  logging.info("Traitement des arêtes de '%s'", facePeau.GetName())

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
        nameFace = "facePipePeau_{}".format(i_aux)
        nameVert = "endEdgeFond_{}".format(i_aux)
        facesPipePeau[i_aux] = face
        endsEdgeFond[i_aux] = sharedVertices[0]
        geomPublish(initLog.debug, face, nameFace, nro_cas)
        geomPublish(initLog.debug, sharedVertices[0], nameVert, nro_cas)
        edgesFace = geompy.ExtractShapes(face, geompy.ShapeType["EDGE"], True)
        for edge in edgesFace:
          if geompy.MinDistance(edge, sharedVertices[0]) < 1e-3:
            edgeRadFacePipePeau[i_aux] = edge
            nameEdge = "edgeRadFacePipePeau_{}".format(i_aux)
            geomPublish(initLog.debug, edge, nameEdge, nro_cas)
            break

  # --- edges elliptiques de la face de peau et points de jonction de la face externe de fissure
  logging.info('Nombre de faces : len(facesPipePeau) = %d', len(facesPipePeau))
  edgesCircPeau = list()
  verticesCircPeau = list()
  for i_aux,fcirc in enumerate(facesPipePeau):
    # Arêtes
    edges = geompy.GetSharedShapesMulti([facePeau, fcirc], geompy.ShapeType["EDGE"])
    groupe = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
    geompy.UnionList(groupe, edges)
    geomPublishInFather(initLog.always, facePeau, groupe, "edgeCirc_{}".format(i_aux))
    edgesCircPeau.append(groupe)
    edgesListees = edgesListees + edges
    # Sommets
    vertices = geompy.GetSharedShapesMulti([facePeau, fcirc], geompy.ShapeType["VERTEX"])
    groupe = geompy.CreateGroup(facePeau, geompy.ShapeType["VERTEX"])
    geompy.UnionList(groupe, vertices)
    geomPublishInFather(initLog.info, facePeau, groupe, "point(s)EdgeCirc_{}".format(i_aux))
    verticesCircPeau.append(groupe)

  logging.info("==> Nombre de groupes d'arêtes   : len(edgesCircPeau)    = %d", len(edgesCircPeau))
  logging.info("==> Nombre de groupes de sommets : len(verticesCircPeau) = %d", len(verticesCircPeau))

  return edgesCircPeau, verticesCircPeau
