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
"""procédure complète de construction d'une fissure longue"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

from . import initLog

from .extractionOrientee import extractionOrientee
from .sortFaces import sortFaces
from .sortEdges import sortEdges
from .eliminateDoubles import eliminateDoubles
from .substractSubShapes import substractSubShapes

# -----------------------------------------------------------------------------

def insereFissureLongue_a(facePorteFissure, WirePorteFondFissure, \
                          fillingFaceExterne, \
                          pipefiss, rayonPipe, \
                          mailleur="MeshGems", nro_cas=None):
  """procedure complete fissure longue"""
  logging.info('start')
  logging.info("Usage du mailleur %s", mailleur)


  # -----------------------------------------------------------------------------
  # --- peau et face de fissure

  # --- partition peau defaut - face de fissure prolongee - wire de fond de fissure prolongée
  partitionPeauFissFond = geompy.MakePartition([facePorteFissure, WirePorteFondFissure, fillingFaceExterne], list(), list(), list(), geompy.ShapeType["FACE"], 0, list(), 0)
  geomPublish(initLog.debug,  partitionPeauFissFond, 'partitionPeauFissFond', nro_cas )

  edges = geompy.ExtractShapes(WirePorteFondFissure, geompy.ShapeType["EDGE"], False)

  lgmax = 0
  imax = 0
  for i_aux, edge in enumerate(edges):
    props = geompy.BasicProperties(edge)
    longueur = props[0]
    if ( longueur > lgmax ):
      lgmax = longueur
      imax = i_aux
  edgemax = edges[imax]
  geomPublish(initLog.debug, edgemax, 'edgemax', nro_cas)
  centreFondFiss = geompy.MakeVertexOnCurve(edgemax, 0.5)
  geomPublish(initLog.debug, centreFondFiss, 'centreFondFiss', nro_cas)
  tangentFondFiss = geompy.MakeTangentOnCurve(edgemax, 0.5)
  geomPublish(initLog.debug, tangentFondFiss, 'tangentFondFiss', nro_cas)

  bord1FondFiss = geompy.MakeVertexOnCurve(edgemax, 0.0)
  geomPublish(initLog.debug, bord1FondFiss, 'bord1FondFiss', nro_cas)
  tangentBord1FondFiss = geompy.MakeTangentOnCurve(edgemax, 0.0)
  geomPublish(initLog.debug, tangentBord1FondFiss, 'tangentBord1FondFiss', nro_cas)

  bord2FondFiss = geompy.MakeVertexOnCurve(edgemax, 1.0)
  geomPublish(initLog.debug, bord2FondFiss, 'bord2FondFiss', nro_cas)
  tangentBord2FondFiss = geompy.MakeTangentOnCurve(edgemax, 1.0)
  geomPublish(initLog.debug, tangentBord2FondFiss, 'tangentBord2FondFiss', nro_cas)

  planBord1 = geompy.MakePlane(bord1FondFiss, tangentBord1FondFiss, 3*rayonPipe)
  planBord2 = geompy.MakePlane(bord2FondFiss, tangentBord2FondFiss, 3*rayonPipe)
  geomPublish(initLog.debug, planBord1, 'planBord1', nro_cas)
  geomPublish(initLog.debug, planBord2, 'planBord2', nro_cas)

  [edgesInside, _, _] = extractionOrientee(fillingFaceExterne, partitionPeauFissFond, centreFondFiss, "EDGE", 1.e-3)
  [facesInside, _, facesOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissFond, centreFondFiss, "FACE", 1.e-3)

  # --- partition peau -face fissure - pipe fond de fissure prolongé
  partitionPeauFissByPipe = geompy.MakePartition([facesInside[0], facesOnside[0]], [pipefiss], list(), list(), geompy.ShapeType["FACE"], 0, list(), 0)
  geomPublish(initLog.always, partitionPeauFissByPipe, 'partitionPeauFissByPipe', nro_cas )

  # --- identification face de peau
  [facesPeauFissInside, _, facesPeauFissOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissByPipe, centreFondFiss, "FACE", 0.1, "peauFiss_bord_")
  facesPeauSorted, _, _ = sortFaces(facesPeauFissOnside) # 4 demi disques, une grande face
  facePeau = facesPeauSorted[-1] # la plus grande face
  geomPublishInFather(initLog.always, partitionPeauFissByPipe, facePeau, "facePeau", nro_cas)

  # --- identification edges de bord face peau
  edgesFilling = geompy.ExtractShapes(fillingFaceExterne, geompy.ShapeType["EDGE"], False)
  edgesBords = list()
  for edge in edgesFilling:
    edgepeau = geompy.GetInPlace(facePeau, edge)
    edgesBords.append(edgepeau)
  groupEdgesBordPeau = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
  geompy.UnionList(groupEdgesBordPeau, edgesBords)
  geomPublishInFather(initLog.always, facePeau, groupEdgesBordPeau , "EdgesBords", nro_cas)

  # --- identification face fissure externe au pipe et edge commune peau fissure
  for face in facesPeauFissInside:
    try:
      sharedEdges = geompy.GetSharedShapesMulti([facePeau, face], geompy.ShapeType["EDGE"])
      if sharedEdges is not None:
        faceFiss = face
        edgePeauFiss = sharedEdges[0]
        geomPublishInFather(initLog.always, partitionPeauFissByPipe, faceFiss, "faceFiss", nro_cas)
        geomPublishInFather(initLog.always, faceFiss, edgePeauFiss, "edgePeauFiss", nro_cas)
        geomPublishInFather(initLog.always, facePeau, edgePeauFiss, "edgePeauFiss", nro_cas)
        break
    except:
      pass
  verticesEdgePeauFiss = geompy.ExtractShapes(edgePeauFiss, geompy.ShapeType["VERTEX"], False)

  # --- identification edges demi cercle dans face de peau
  edgesFacePeau = geompy.ExtractShapes(facePeau, geompy.ShapeType["EDGE"], False)
  edgesFacePeauSorted, _, _ = sortEdges(edgesFacePeau)
  demiCerclesPeau = edgesFacePeauSorted[0:4]
  verticesDemiCerclesPeau = list()
  for i_aux, edge in enumerate(demiCerclesPeau):
    name = "demiCerclePeau_{}".format(i_aux)
    geomPublishInFather(initLog.debug,facePeau, edge, name, nro_cas)
    verticesDemiCerclesPeau += geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], False)
  verticesDemiCerclesPeau = eliminateDoubles(facePeau, verticesDemiCerclesPeau)
  for i_aux, vertex in enumerate(verticesDemiCerclesPeau):
    name = "verticesDemiCerclesPeau_{}".format(i_aux)
    geomPublishInFather(initLog.debug,facePeau, vertex, name, nro_cas)
  verticesOutCercles = substractSubShapes(facePeau, verticesDemiCerclesPeau, verticesEdgePeauFiss)
  for i_aux, vertex in enumerate(verticesOutCercles):
    name = "verticesOutCercles_{}".format(i_aux)
    geomPublishInFather(initLog.debug,facePeau, vertex, name, nro_cas)

  # --- demi cercles  regroupés
  groupsDemiCerclesPeau = list()
  for i_aux, vertex in enumerate(verticesEdgePeauFiss):
    demis = list()
    for edge in demiCerclesPeau:
      if ( geompy.MinDistance(vertex, edge) < 1.e-5 ):
        demis.append(edge)
    group = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
    geompy.UnionList(group, demis)
    name = "Cercle{}".format(i_aux)
    geomPublishInFather(initLog.always,facePeau, group , name, nro_cas)
    groupsDemiCerclesPeau.append(group)

  # --- identification edges commune pipe face fissure externe au pipe
  edgePeauFissId = geompy.GetSubShapeID(partitionPeauFissByPipe, edgePeauFiss)
  edgesFaceFiss = geompy.ExtractShapes(faceFiss, geompy.ShapeType["EDGE"], False)
  edgesFaceFissPipe = list()
  for edge in edgesFaceFiss:
    if geompy.GetSubShapeID(partitionPeauFissByPipe, edge) != edgePeauFissId:
      edgesFaceFissPipe.append(edge)
      name = "edgeFaceFissPipe_%d"%len(edgesFaceFissPipe)
      geomPublishInFather(initLog.debug,faceFiss, edge, name, nro_cas)
  groupEdgesFaceFissPipe = geompy.CreateGroup(faceFiss, geompy.ShapeType["EDGE"])
  geompy.UnionList(groupEdgesFaceFissPipe, edgesFaceFissPipe)
  geomPublishInFather(initLog.debug,faceFiss, groupEdgesFaceFissPipe, "edgesFaceFissPipe", nro_cas)

  return edgesInside, centreFondFiss, tangentFondFiss, \
          planBord1, planBord2, \
          facePeau, faceFiss, verticesOutCercles, verticesEdgePeauFiss, \
          edgePeauFiss, demiCerclesPeau, \
          groupEdgesBordPeau, groupsDemiCerclesPeau, groupEdgesFaceFissPipe
