# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
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

import os

import logging
import salome
from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog
from .geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH
import math

from .extractionOrientee import extractionOrientee
from .sortFaces import sortFaces
from .sortEdges import sortEdges
from .eliminateDoubles import eliminateDoubles
from .substractSubShapes import substractSubShapes
from .produitMixte import produitMixte
from .findWireEndVertices import findWireEndVertices
from .getSubshapeIds import getSubshapeIds
from .putName import putName
from .distance2 import distance2
from .enleveDefaut import enleveDefaut
from .shapeSurFissure import shapeSurFissure
from .regroupeSainEtDefaut import RegroupeSainEtDefaut
from .triedreBase import triedreBase

from .insereFissureLongue_a import insereFissureLongue_a
from .insereFissureLongue_b import insereFissureLongue_b
from .insereFissureLongue_c import insereFissureLongue_c
from .insereFissureLongue_d import insereFissureLongue_d

# -----------------------------------------------------------------------------

def insereFissureLongue(geometriesSaines, \
                        shapesFissure, shapeFissureParams, \
                        maillageFissureParams, elementsDefaut, \
                        step=-1, mailleur="MeshGems"):
  """procedure complete fissure longue"""
  logging.info('start')
  logging.info("Usage du mailleur %s", mailleur)

  #geometrieSaine    = geometriesSaines[0]
  shapeDefaut       = shapesFissure[0] # face de fissure, debordant
  #tailleDefaut      = shapesFissure[2]
  wiretube          = shapesFissure[4] # wire fond de fissure, debordant
  planfiss          = shapesFissure[7] # plan de fissure
  pipefiss          = shapesFissure[8] # pipe, debordant

  profondeur  = shapeFissureParams['profondeur']
  rayonPipe   = profondeur/4.0

  nomRep            = maillageFissureParams['nomRep']
  nomFicSain        = maillageFissureParams['nomFicSain']
  nomFicFissure     = maillageFissureParams['nomFicFissure']

  #nbsegExt          = maillageFissureParams['nbsegExt']      # 5
  #nbsegGen          = maillageFissureParams['nbsegGen']      # 25
  #nbsegRad          = maillageFissureParams['nbsegRad']      # 5
  #scaleRad          = maillageFissureParams['scaleRad']      # 4
  #nbsegCercle       = maillageFissureParams['nbsegCercle']   # 6
  #nbsegFis          = maillageFissureParams['nbsegFis']      # 20
  #lensegEllipsoide  = maillageFissureParams['lensegEllipso'] # 1.0

  #fichierMaillageSain = os.path.join(nomRep, '{}.med'.format(nomFicSain))
  fichierMaillageFissure = os.path.join(nomRep, '{}.med'.format(nomFicFissure))

  facesDefaut              = elementsDefaut[0]
  #centreDefaut             = elementsDefaut[1]
  #normalDefaut             = elementsDefaut[2]
  #extrusionDefaut          = elementsDefaut[3]
  #dmoyen                   = elementsDefaut[4]
  #bordsPartages            = elementsDefaut[5]
  #fillconts                = elementsDefaut[6]
  #idFilToCont              = elementsDefaut[7]
  maillageSain             = elementsDefaut[8]
  internalBoundary         = elementsDefaut[9]
  zoneDefaut               = elementsDefaut[10]
  zoneDefaut_skin          = elementsDefaut[11]
  zoneDefaut_internalFaces = elementsDefaut[12]
  zoneDefaut_internalEdges = elementsDefaut[13]

  facePorteFissure =  shapeDefaut
  WirePorteFondFissure = wiretube
  fillingFaceExterne = facesDefaut[0]
  logging.debug("fillingFaceExterne %s", fillingFaceExterne)
  geomPublish(initLog.debug, fillingFaceExterne, "fillingFaceExterne")
  edgesFilling = geompy.ExtractShapes(fillingFaceExterne, geompy.ShapeType["EDGE"], False)

  O, OX, OY, OZ = triedreBase()

  # -----------------------------------------------------------------------------
  # --- peau et face de fissure

  # --- partition peau defaut - face de fissure prolongee - wire de fond de fissure prolongée
  partitionPeauFissFond = geompy.MakePartition([facePorteFissure, WirePorteFondFissure, fillingFaceExterne], list(), list(), list(), geompy.ShapeType["FACE"], 0, list(), 0)
  geomPublish(initLog.debug,  partitionPeauFissFond, 'partitionPeauFissFond' )

  edges = geompy.ExtractShapes(WirePorteFondFissure, geompy.ShapeType["EDGE"], False)

  lgmax = 0
  imax = 0
  for i, edge in enumerate(edges):
    props = geompy.BasicProperties(edge)
    lg = props[0]
    if lg > lgmax:
      lgmax = lg
      imax = i
  edgemax = edges[imax]
  geomPublish(initLog.debug, edgemax, 'edgemax')
  centreFondFiss = geompy.MakeVertexOnCurve(edgemax, 0.5)
  geomPublish(initLog.debug, centreFondFiss, 'centreFondFiss')
  tangentFondFiss = geompy.MakeTangentOnCurve(edgemax, 0.5)
  geomPublish(initLog.debug, tangentFondFiss, 'tangentFondFiss')

  bord1FondFiss = geompy.MakeVertexOnCurve(edgemax, 0.0)
  geomPublish(initLog.debug, bord1FondFiss, 'bord1FondFiss')
  tangentBord1FondFiss = geompy.MakeTangentOnCurve(edgemax, 0.0)
  geomPublish(initLog.debug, tangentBord1FondFiss, 'tangentBord1FondFiss')

  bord2FondFiss = geompy.MakeVertexOnCurve(edgemax, 1.0)
  geomPublish(initLog.debug, bord2FondFiss, 'bord2FondFiss')
  tangentBord2FondFiss = geompy.MakeTangentOnCurve(edgemax, 1.0)
  geomPublish(initLog.debug, tangentBord2FondFiss, 'tangentBord2FondFiss')

  planBord1 = geompy.MakePlane(bord1FondFiss, tangentBord1FondFiss, 3*rayonPipe)
  planBord2 = geompy.MakePlane(bord2FondFiss, tangentBord2FondFiss, 3*rayonPipe)
  geomPublish(initLog.debug, planBord1, 'planBord1')
  geomPublish(initLog.debug, planBord2, 'planBord2')

  [edgesInside, edgesOutside, edgesOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissFond, centreFondFiss, "EDGE", 1.e-3)
  [facesInside, facesOutside, facesOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissFond, centreFondFiss, "FACE", 1.e-3)

  # --- partition peau -face fissure - pipe fond de fissure prolongé
  partitionPeauFissByPipe = geompy.MakePartition([facesInside[0], facesOnside[0]], [pipefiss], list(), list(), geompy.ShapeType["FACE"], 0, list(), 0)
  geomPublish(initLog.debug,  partitionPeauFissByPipe, 'partitionPeauFissByPipe' )

  # --- identification face de peau
  [facesPeauFissInside, facesPeauFissOutside, facesPeauFissOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissByPipe, centreFondFiss, "FACE", 0.1, "peauFiss_bord_")
  facesPeauSorted, minsur, maxsurf = sortFaces(facesPeauFissOnside) # 4 demi disques, une grande face
  facePeau = facesPeauSorted[-1] # la plus grande face
  geomPublishInFather(initLog.debug,partitionPeauFissByPipe, facePeau, "facePeau")

  # --- identification edges de bord face peau
  edgesBords = list()
  for i, edge in enumerate(edgesFilling):
    edgepeau = geompy.GetInPlace(facePeau, edge)
    edgesBords.append(edgepeau)
  groupEdgesBordPeau = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
  geompy.UnionList(groupEdgesBordPeau, edgesBords)
  geomPublishInFather(initLog.debug,facePeau, groupEdgesBordPeau , "EdgesBords")

  # --- identification face fissure externe au pipe et edge commune peau fissure
  for face in facesPeauFissInside:
    try:
      sharedEdges = geompy.GetSharedShapesMulti([facePeau, face], geompy.ShapeType["EDGE"])
      if sharedEdges is not None:
        faceFiss = face
        edgePeauFiss = sharedEdges[0]
        geomPublishInFather(initLog.debug,partitionPeauFissByPipe, faceFiss, "faceFiss")
        geomPublishInFather(initLog.debug,faceFiss, edgePeauFiss, "edgePeauFiss")
        geomPublishInFather(initLog.debug,facePeau, edgePeauFiss, "edgePeauFiss")
        break
    except:
      pass
  verticesEdgePeauFiss = geompy.ExtractShapes(edgePeauFiss, geompy.ShapeType["VERTEX"], False)

  # --- identification edges demi cercle dans face de peau
  edgesFacePeau = geompy.ExtractShapes(facePeau, geompy.ShapeType["EDGE"], False)
  edgesFacePeauSorted, minlg, maxlg = sortEdges(edgesFacePeau)
  demiCerclesPeau = edgesFacePeauSorted[0:4]
  verticesDemiCerclesPeau = list()
  for i, edge in enumerate(demiCerclesPeau):
    name = "demiCerclePeau_%d"%i
    geomPublishInFather(initLog.debug,facePeau, edge, name)
    verticesDemiCerclesPeau += geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], False)
  verticesDemiCerclesPeau = eliminateDoubles(facePeau, verticesDemiCerclesPeau)
  for i, vertex in enumerate(verticesDemiCerclesPeau):
    name = "verticesDemiCerclesPeau_%d"%i
    geomPublishInFather(initLog.debug,facePeau, vertex, name)
  verticesOutCercles = substractSubShapes(facePeau, verticesDemiCerclesPeau, verticesEdgePeauFiss)
  for i, vertex in enumerate(verticesOutCercles):
    name = "verticesOutCercles_%d"%i
    geomPublishInFather(initLog.debug,facePeau, vertex, name)

  # --- demi cercles  regroupés
  groupsDemiCerclesPeau = list()
  for i, vertex in enumerate(verticesEdgePeauFiss):
    demis = list()
    for edge in demiCerclesPeau:
      if geompy.MinDistance(vertex, edge) < 1.e-5:
        demis.append(edge)
    group = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
    geompy.UnionList(group, demis)
    name = "Cercle%d"%i
    geomPublishInFather(initLog.debug,facePeau, group , name)
    groupsDemiCerclesPeau.append(group)

  # --- identification edges commune pipe face fissure externe au pipe
  edgePeauFissId = geompy.GetSubShapeID(partitionPeauFissByPipe, edgePeauFiss)
  edgesFaceFiss = geompy.ExtractShapes(faceFiss, geompy.ShapeType["EDGE"], False)
  edgesFaceFissPipe = list()
  for edge in edgesFaceFiss:
    if geompy.GetSubShapeID(partitionPeauFissByPipe, edge) != edgePeauFissId:
      edgesFaceFissPipe.append(edge)
      name = "edgeFaceFissPipe_%d"%len(edgesFaceFissPipe)
      geomPublishInFather(initLog.debug,faceFiss, edge, name)
  groupEdgesFaceFissPipe = geompy.CreateGroup(faceFiss, geompy.ShapeType["EDGE"])
  geompy.UnionList(groupEdgesFaceFissPipe, edgesFaceFissPipe)
  geomPublishInFather(initLog.debug,faceFiss, groupEdgesFaceFissPipe, "edgesFaceFissPipe")

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
  pm = produitMixte(centreFondFiss, vertex, vertproj, sommetAxe)
  if pm > 0:
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
  geomPublish(initLog.debug,  wireFondFiss, 'wireFondFiss')
  geomPublish(initLog.debug,  pipeFondFiss, 'pipeFondFiss')

  VerticesEndFondFiss = findWireEndVertices(wireFondFiss)
  for i, v in enumerate(VerticesEndFondFiss):
    name = "vertexEndFondFiss_%d"%i
    geomPublishInFather(initLog.debug,wireFondFiss, v, name)
  VerticesEndPipeFiss = list()
  for v in VerticesEndFondFiss:
    VerticesEndPipeFiss.append(geompy.GetInPlace(pipeFondFiss, v))
  for i, v in enumerate(VerticesEndPipeFiss):
    name = "vertexEndPipeFiss_%d"%i
    geomPublishInFather(initLog.debug,pipeFondFiss, v, name)

  geomPublishInFather(initLog.debug,pipeFondFiss, VerticesEndPipeFiss[0], "PFOR")
  geomPublishInFather(initLog.debug,pipeFondFiss, VerticesEndPipeFiss[1], "PFEX")
  if geompy.MinDistance(VerticesEndPipeFiss[0], verticesOutCercles[0]) > geompy.MinDistance(VerticesEndPipeFiss[0], verticesOutCercles[1]):
    a = verticesOutCercles[0]
    verticesOutCercles[0] = verticesOutCercles[1]
    verticesOutCercles[1] = a
  geomPublishInFather(initLog.debug,facePeau, verticesOutCercles[0], "THOR")
  geomPublishInFather(initLog.debug,facePeau, verticesOutCercles[1], "THEX")

  [facesPipeInside, facesPipeOutside, facesPipeOnside] = extractionOrientee(fillingFaceExterne, pipeFondFiss, centreFondFiss, "FACE", 0.1, "pipe_bord_")
  [edgesPipeInside, edgesPipeOutside, edgesPipeOnside] = extractionOrientee(fillingFaceExterne, pipeFondFiss, centreFondFiss, "EDGE", 0.1, "pipe_bord_")
  disqueInt1 = geompy.GetInPlaceByHistory(pipeFondFiss, planBord1)
  disqueInt2 = geompy.GetInPlaceByHistory(pipeFondFiss, planBord2)
  disques = facesPipeOnside + [disqueInt1, disqueInt2]
  edgesDiskInt = geompy.ExtractShapes(disqueInt1, geompy.ShapeType["EDGE"], False)
  edgesDiskInt = edgesDiskInt +geompy.ExtractShapes(disqueInt2, geompy.ShapeType["EDGE"], False)
  edgesSorted, minlg, maxlg = sortEdges(edgesDiskInt) # 4 rayons, 2 demi cercles

  centre = geompy.MakeVertexOnSurface(planfiss, 0.5, 0.5)
  refpoint = geompy.MakeTranslationVector(centre, geompy.GetNormal(planfiss,centre))
  geomPublish(initLog.debug, refpoint, 'refpoint')
  [facesPipeInplan, facesPipeOutplan, facesPipeOnplan] = extractionOrientee(planfiss, pipeFondFiss, refpoint, "FACE", 0.1, "pipe_plan_")
  [edgesPipeInplan, edgesPipeOutplan, edgesPipeOnplan] = extractionOrientee(planfiss, pipeFondFiss, refpoint, "EDGE", 0.1, "pipe_plan_")

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

  aFilterManager = smesh.CreateFilterManager()
  nbAdded, internalBoundary, _NoneGroup = internalBoundary.MakeBoundaryElements( SMESH.BND_1DFROM2D, '', '', 0, [  ])
  criteres = list()
  unCritere = smesh.GetCriterion(SMESH.EDGE,SMESH.FT_FreeBorders,SMESH.FT_Undefined,0)
  criteres.append(unCritere)
  filtre = smesh.GetFilterFromCriteria(criteres)
  bordsLibres = internalBoundary.MakeGroupByFilter( 'bords', filtre )
  smesh.SetName(bordsLibres, 'bordsLibres')

  # --- pour aider l'algo hexa-tetra a ne pas mettre de pyramides a l'exterieur des volumes replies sur eux-memes
  #     on designe les faces de peau en quadrangles par le groupe "skinFaces"

  skinFaces = internalBoundary.CreateEmptyGroup( SMESH.FACE, 'skinFaces' )
  nbAdd = skinFaces.AddFrom( internalBoundary.GetMesh() )

  # --- maillage pipe fond fissure

  meshFondFiss, groups_demiCercles, group_generFiss, nbSegGenLong, nbSegGenBout = \
            insereFissureLongue_a (pipeFondFiss, disques, rayons, demiCercles, demiCerclesPeau, generatrices, \
            VerticesEndPipeFiss, verticesEdgePeauFiss, \
            groupFaceFissInPipe, groupEdgeFondFiss, groupsDemiCerclesPipe, groupGenerFiss, \
            profondeur, rayonPipe, distance2)

  # --- maillage face de peau

  meshFacePeau, groupEdgesPeauFiss = \
            insereFissureLongue_b ( facePeau, edgePeauFiss, groupEdgesBordPeau, bordsLibres, \
                                    groupsDemiCerclesPeau, groups_demiCercles, verticesOutCercles, \
                                    nbSegGenLong, nbSegGenBout, profondeur, \
                                    mailleur )

  # --- maillage face de fissure

  meshFaceFiss = \
            insereFissureLongue_c ( faceFiss, edgePeauFiss, groupEdgesPeauFiss, group_generFiss, groupEdgesFaceFissPipe, \
                                    profondeur, rayonPipe, \
                                    mailleur )

  # --- maillage meshBoiteDefaut

  meshBoiteDefaut, group_faceFissInPipe, group_faceFissOutPipe = \
            insereFissureLongue_d ( internalBoundary, meshFondFiss, meshFacePeau, meshFaceFiss, \
                                    mailleur )


  groups = maillageSain.GetGroups()
  grps1 = [ grp for grp in groups if grp.GetName() == 'P1']
  grps2 = [ grp for grp in groups if grp.GetName() == 'P2']
  coords1 = maillageSain.GetNodeXYZ(grps1[0].GetID(1))
  coords2 = maillageSain.GetNodeXYZ(grps2[0].GetID(1))
  logging.info("coords1 %s, coords2 %s",coords1, coords2)

  faceFissure = meshBoiteDefaut.GetMesh().UnionListOfGroups( [ group_faceFissOutPipe, group_faceFissInPipe ], 'FACE1' )
  maillageSain = enleveDefaut(maillageSain, zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges)
  putName(maillageSain, nomFicSain+"_coupe")
  extrusionFaceFissure, normfiss = shapeSurFissure(facePorteFissure)
  maillageComplet = RegroupeSainEtDefaut(maillageSain, meshBoiteDefaut, extrusionFaceFissure, facePorteFissure, 'COUDE')

  groups = maillageComplet.GetGroups()
  grps1 = [ grp for grp in groups if grp.GetName() == 'P1']
  grps2 = [ grp for grp in groups if grp.GetName() == 'P2']
  nodeid1 = maillageComplet.AddNode(coords1[0], coords1[1], coords1[2])
  nodeid2 = maillageComplet.AddNode(coords2[0], coords2[1], coords2[2])
  grps1[0].Add([nodeid1])
  grps2[0].Add([nodeid2])
  ma0d1 = maillageComplet.Add0DElement(nodeid1)
  ma0d2 = maillageComplet.Add0DElement(nodeid2)
  grpma0d1 = maillageComplet.CreateEmptyGroup( SMESH.ELEM0D, 'P1' )
  nbAdd = grpma0d1.Add( [ma0d1] )
  grpma0d2 = maillageComplet.CreateEmptyGroup( SMESH.ELEM0D, 'P2' )
  nbAdd = grpma0d2.Add( [ma0d2] )

#  grps = [ grp for grp in groups if grp.GetName() == 'affectedEdges']
#  grps[0].SetName('affEdges')
#  grps = [ grp for grp in groups if grp.GetName() == 'affectedFaces']
#  grps[0].SetName('affFaces')
#  grps = [ grp for grp in groups if grp.GetName() == 'affectedVolumes']
#  grps[0].SetName('affVols')

  maillageComplet.ConvertToQuadratic( 1 )
  grps = [ grp for grp in groups if grp.GetName() == 'FONDFISS']
  fond = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FONDFISS' )

  grps = [ grp for grp in groups if grp.GetName() == 'FACE1']
  nb = maillageComplet.Reorient2D( grps[0], normfiss, grps[0].GetID(1))

  plansim = geompy.MakePlane(O, normfiss, 10000)
  fissnorm = geompy.MakeMirrorByPlane(normfiss, plansim)
  grps = [ grp for grp in groups if grp.GetName() == 'FACE2']
  nb = maillageComplet.Reorient2D( grps[0], fissnorm, grps[0].GetID(1))
  #isDone = maillageComplet.ReorientObject( grps[0] )
  fond = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FACE2' )

  maillageComplet.ExportMED(fichierMaillageFissure)
  putName(maillageComplet, nomFicFissure)
  logging.info("fichier maillage fissure %s", fichierMaillageFissure)

  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser()

  return  maillageComplet
