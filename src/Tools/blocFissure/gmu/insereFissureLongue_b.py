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

import SMESH

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from .geomsmesh import smesh

from . import initLog

from .extractionOrientee import extractionOrientee
from .sortEdges import sortEdges
from .produitMixte import produitMixte
from .findWireEndVertices import findWireEndVertices
from .getSubshapeIds import getSubshapeIds
from .putName import putName

# -----------------------------------------------------------------------------

def insereFissureLongue_b (edgesInside, centreFondFiss, tangentFondFiss, \
                           planfiss, planBord1, planBord2, \
                           facePeau, verticesOutCercles, verticesEdgePeauFiss, \
                           fillingFaceExterne, rayonPipe, \
                           internalBoundary, \
                           nro_cas=None):
  """procedure complete fissure longue"""
  logging.info('start')
  logging.info("Maillage pour le cas n°%s", nro_cas)

  # -----------------------------------------------------------------------------
  # --- pipe de fond de fissure

  wireFondFiss = geompy.MakeWire(edgesInside, 1e-07)

  disque = geompy.MakeDiskPntVecR(centreFondFiss, tangentFondFiss, rayonPipe)
  [vertex] = geompy.ExtractShapes(disque, geompy.ShapeType["VERTEX"], False)
  vertproj = geompy.MakeProjection(vertex, planfiss)
  vec1 = geompy.MakeVector(centreFondFiss, vertex)
  try:
    # si centreFondFiss et vertproj sont proches: exception. Angle = +- 90°
    vec2 = geompy.MakeVector(centreFondFiss, vertproj)
    angle = geompy.GetAngleRadians(vec1, vec2)
  except:
    # on utilise la projection du centre sur la peau pour avoir un vecteur non nul
    vertproj = geompy.MakeProjection(centreFondFiss, facePeau)
    vec2 = geompy.MakeVector(centreFondFiss, vertproj)
    angle = geompy.GetAngleRadians(vec1, vec2)
  sommetAxe = geompy.MakeTranslationVector(centreFondFiss, tangentFondFiss)
  if ( produitMixte(centreFondFiss, vertex, vertproj, sommetAxe) > 0 ):
    disque = geompy.MakeRotation(disque, tangentFondFiss, angle)
  else:
    disque = geompy.MakeRotation(disque, tangentFondFiss, -angle)
  [vertexReference] = geompy.ExtractShapes(disque, geompy.ShapeType["VERTEX"], False)

  pipeFondFiss = geompy.MakePipe(disque, wireFondFiss)
  pipeFondFiss = geompy.MakePartition([pipeFondFiss], [planfiss, wireFondFiss, planBord1, planBord2], list(), list(), geompy.ShapeType["SOLID"], 0, list(), 0)
  #pipe = geompy.MakePipe(disque, WirePorteFondFissure)
  #pipe = geompy.MakePartition([pipe],[fillingFaceExterne], list(), list(), geompy.ShapeType["SOLID"], 0, list(), 0)
  #pipes = geompy.ExtractShapes(pipe, geompy.ShapeType["SOLID"], False)
  #pipesSorted, volmin, volmax = sortSolids(pipes)
  #pipeFondFiss = pipesSorted[-1]
  #pipeFondFiss = geompy.MakePartition([pipeFondFiss], [planfiss, wireFondFiss, planBord1, planBord2], list(), list(), geompy.ShapeType["SOLID"], 0, list(), 0)

  geomPublish(initLog.debug,  disque, 'disque')
  geomPublish(initLog.always,  wireFondFiss, 'wireFondFiss', nro_cas)
  geomPublish(initLog.always,  pipeFondFiss, 'pipeFondFiss', nro_cas)

  VerticesEndFondFiss, _ = findWireEndVertices(wireFondFiss)
  for i_aux, vertex in enumerate(VerticesEndFondFiss):
    name = "vertexEndFondFiss_{}".format(i_aux)
    geomPublishInFather(initLog.debug,wireFondFiss, vertex, name)
  VerticesEndPipeFiss = list()
  for vertex in VerticesEndFondFiss:
    VerticesEndPipeFiss.append(geompy.GetInPlace(pipeFondFiss, vertex))
  for i_aux, vertex in enumerate(VerticesEndPipeFiss):
    name = "vertexEndPipeFiss_{}".format(i_aux)
    geomPublishInFather(initLog.debug,pipeFondFiss, vertex, name)

  geomPublishInFather(initLog.debug,pipeFondFiss, VerticesEndPipeFiss[0], "PFOR")
  geomPublishInFather(initLog.debug,pipeFondFiss, VerticesEndPipeFiss[1], "PFEX")
  if ( geompy.MinDistance(VerticesEndPipeFiss[0], verticesOutCercles[0]) > geompy.MinDistance(VerticesEndPipeFiss[0], verticesOutCercles[1]) ):
    aux = verticesOutCercles[0]
    verticesOutCercles[0] = verticesOutCercles[1]
    verticesOutCercles[1] = aux
  geomPublishInFather(initLog.debug,facePeau, verticesOutCercles[0], "THOR")
  geomPublishInFather(initLog.debug,facePeau, verticesOutCercles[1], "THEX")

  [_, _, facesPipeOnside] = extractionOrientee(fillingFaceExterne, pipeFondFiss, centreFondFiss, "FACE", 0.1, "pipe_bord_")
  [_, _, edgesPipeOnside] = extractionOrientee(fillingFaceExterne, pipeFondFiss, centreFondFiss, "EDGE", 0.1, "pipe_bord_")
  disqueInt1 = geompy.GetInPlaceByHistory(pipeFondFiss, planBord1)
  disqueInt2 = geompy.GetInPlaceByHistory(pipeFondFiss, planBord2)
  disques = facesPipeOnside + [disqueInt1, disqueInt2]
  edgesDiskInt = geompy.ExtractShapes(disqueInt1, geompy.ShapeType["EDGE"], False)
  edgesDiskInt = edgesDiskInt +geompy.ExtractShapes(disqueInt2, geompy.ShapeType["EDGE"], False)
  edgesSorted, _, _ = sortEdges(edgesDiskInt) # 4 rayons, 2 demi cercles

  centre = geompy.MakeVertexOnSurface(planfiss, 0.5, 0.5)
  refpoint = geompy.MakeTranslationVector(centre, geompy.GetNormal(planfiss,centre))
  geomPublish(initLog.debug, refpoint, 'refpoint')
  [_, _, facesPipeOnplan] = extractionOrientee(planfiss, pipeFondFiss, refpoint, "FACE", 0.1, "pipe_plan_")
  [_, _, edgesPipeOnplan] = extractionOrientee(planfiss, pipeFondFiss, refpoint, "EDGE", 0.1, "pipe_plan_")

  # --- rayon disques = (edgesPipeOnside inter edgesPipeOnplan) + rayons disque internes
  #     demi cercles  = edgesPipeOnside moins edgesPipeOnplan + demi cercles disque internes
  #     generatrices  = edgesPipeOnplan moins rayon disques (3 grandes et 6 petites)
  edgesIdPipeOnside = getSubshapeIds(pipeFondFiss, edgesPipeOnside)
  edgesIdPipeOnplan = getSubshapeIds(pipeFondFiss, edgesPipeOnplan)
  rayons = list()
  demiCercles = list()
  for i, edgeId in enumerate(edgesIdPipeOnside):
    if edgeId in edgesIdPipeOnplan:
      rayons.append(edgesPipeOnside[i])
    else:
      demiCercles.append(edgesPipeOnside[i])
  demiCerclesExternes = demiCercles
  rayons = rayons + edgesSorted[:4]            # les 4 plus petits sont les rayons
  demiCercles = demiCercles  + edgesSorted[4:] # les suivants sont les arcs de cercle
  rayonsId = getSubshapeIds(pipeFondFiss, rayons)
  generatrices = list()
  for i, edgeId in enumerate(edgesIdPipeOnplan):
    if edgeId not in rayonsId:
      generatrices.append(edgesPipeOnplan[i])

  # --- generatrices en contact avec la face fissure externe au pipe
  generFiss = list()
  for edge in generatrices:
    distance = geompy.MinDistance(vertexReference, edge)
    logging.debug("distance %s", distance)
    if distance < 1.e-5:
      generFiss.append(edge)
      break
  for edge in generatrices:
    distance = geompy.MinDistance(generFiss[0], edge)
    logging.debug("distance %s", distance)
    if distance < 1.e-5:
      generFiss.append(edge)
  groupGenerFiss = geompy.CreateGroup(pipeFondFiss, geompy.ShapeType["EDGE"])
  geompy.UnionList(groupGenerFiss, generFiss)
  geomPublishInFather(initLog.debug,pipeFondFiss, groupGenerFiss, "GenFiss")

  # --- demi cercles externes regroupés
  groupsDemiCerclesPipe = list()
  for i, vertex in enumerate(verticesEdgePeauFiss):
    demis = list()
    for edge in demiCerclesExternes:
      if geompy.MinDistance(vertex, edge) < 0.1:
        demis.append(edge)
    group = geompy.CreateGroup(pipeFondFiss, geompy.ShapeType["EDGE"])
    geompy.UnionList(group, demis)
    name = "Cercle%d"%i
    geomPublishInFather(initLog.debug,pipeFondFiss, group , name)
    groupsDemiCerclesPipe.append(group)

  # --- faces fissure dans le pipe

  facesFissinPipe = list()
  generFissId = getSubshapeIds(pipeFondFiss, generFiss)
  logging.debug("generatrice fissure %s", generFissId)
  for face in facesPipeOnplan:
    edges =geompy.ExtractShapes(face, geompy.ShapeType["EDGE"], False)
    edgesId = getSubshapeIds(pipeFondFiss, edges)
    logging.debug("  edges %s", edgesId)
    for i,edgeId in enumerate(edgesId):
      if edgeId in generFissId:
        logging.debug("face found")
        facesFissinPipe.append(face)
        name = "faceFissInPipe_%d"%i
        geomPublishInFather(initLog.debug,pipeFondFiss, face, name)
        break
  groupFaceFissInPipe = geompy.CreateGroup(pipeFondFiss, geompy.ShapeType["FACE"])
  geompy.UnionList(groupFaceFissInPipe, facesFissinPipe)
  name = "FaceFissInPipe"
  geomPublishInFather(initLog.debug,pipeFondFiss, groupFaceFissInPipe , name)

  # --- edges de fond de fissure

  edgesFondFiss = list()
  for i, edge in enumerate(edgesInside):
    anEdge = geompy.GetInPlace(pipeFondFiss, edge)
    logging.debug("  edge %s ", anEdge)
    edgesFondFiss.append(anEdge)
    name ="edgeFondFissure_%d"%i
    geomPublishInFather(initLog.debug,pipeFondFiss, anEdge, name)
  groupEdgeFondFiss = geompy.CreateGroup(pipeFondFiss, geompy.ShapeType["EDGE"])
  geompy.UnionList(groupEdgeFondFiss, edgesFondFiss)
  name = "FONDFISS"
  geomPublishInFather(initLog.debug,pipeFondFiss, groupEdgeFondFiss , name)

  # -------------------------------------------------------------------------
  # --- maillage

  # --- edges de bord face defaut à respecter

  _ = smesh.CreateFilterManager()
  _, internalBoundary, _ = internalBoundary.MakeBoundaryElements( SMESH.BND_1DFROM2D, '', '', 0, [  ])
  putName(internalBoundary, 'internalBoundary', i_pref=nro_cas)
  criteres = list()
  un_critere = smesh.GetCriterion(SMESH.EDGE,SMESH.FT_FreeBorders,SMESH.FT_Undefined,0)
  criteres.append(un_critere)
  filtre = smesh.GetFilterFromCriteria(criteres)
  bordsLibres = internalBoundary.MakeGroupByFilter( 'bords', filtre )
  putName(bordsLibres, 'bordsLibres', i_pref=nro_cas)

  # --- pour aider l'algo hexa-tetra a ne pas mettre de pyramides a l'exterieur des volumes replies sur eux-memes
  #     on designe les faces de peau en quadrangles par le groupe "skinFaces"

  skinFaces = internalBoundary.CreateEmptyGroup( SMESH.FACE, 'skinFaces' )
  _ = skinFaces.AddFrom( internalBoundary.GetMesh() )

  # --- maillage pipe fond fissure

  return pipeFondFiss, disques, rayons, \
          demiCercles, generatrices, \
          VerticesEndPipeFiss, bordsLibres, \
          groupFaceFissInPipe, groupEdgeFondFiss, groupsDemiCerclesPipe, groupGenerFiss
