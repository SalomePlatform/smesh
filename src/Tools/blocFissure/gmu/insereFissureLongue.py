# -*- coding: utf-8 -*-

import logging
import salome
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
from geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH
import math

from extractionOrientee import extractionOrientee
from sortFaces import sortFaces
from sortEdges import sortEdges
from eliminateDoubles import eliminateDoubles
from substractSubShapes import substractSubShapes
from produitMixte import produitMixte
from findWireEndVertices import findWireEndVertices
from getSubshapeIds import getSubshapeIds
from putName import putName
from distance2 import distance2
from enleveDefaut import enleveDefaut
from shapeSurFissure import shapeSurFissure
from regroupeSainEtDefaut import RegroupeSainEtDefaut
from triedreBase import triedreBase

# -----------------------------------------------------------------------------
# --- procedure complete fissure longue

def insereFissureLongue(geometriesSaines, maillagesSains,
                        shapesFissure, shapeFissureParams,
                        maillageFissureParams, elementsDefaut, step=-1):
  """
  TODO: a completer
  """
  logging.info('start')

  #geometrieSaine    = geometriesSaines[0]
  #maillageSain      = maillagesSains[0]
  #isHexa            = maillagesSains[1]
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

  #fichierMaillageSain = nomRep + '/' + nomFicSain + '.med'
  fichierMaillageFissure = nomRep + '/' + nomFicFissure + '.med'

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
  partitionPeauFissFond = geompy.MakePartition([facePorteFissure, WirePorteFondFissure, fillingFaceExterne], [], [], [], geompy.ShapeType["FACE"], 0, [], 0)
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
  partitionPeauFissByPipe = geompy.MakePartition([facesInside[0], facesOnside[0]], [pipefiss], [], [], geompy.ShapeType["FACE"], 0, [], 0)
  geomPublish(initLog.debug,  partitionPeauFissByPipe, 'partitionPeauFissByPipe' )

  # --- identification face de peau
  [facesPeauFissInside, facesPeauFissOutside, facesPeauFissOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissByPipe, centreFondFiss, "FACE", 0.1, "peauFiss_bord_")
  facesPeauSorted, minsur, maxsurf = sortFaces(facesPeauFissOnside) # 4 demi disques, une grande face
  facePeau = facesPeauSorted[-1] # la plus grande face
  geomPublishInFather(initLog.debug,partitionPeauFissByPipe, facePeau, "facePeau")

  # --- identification edges de bord face peau
  edgesBords = []
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
  verticesDemiCerclesPeau = []
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
  groupsDemiCerclesPeau = []
  for i, vertex in enumerate(verticesEdgePeauFiss):
    demis = []
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
  edgesFaceFissPipe = []
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
  pipeFondFiss = geompy.MakePartition([pipeFondFiss], [planfiss, wireFondFiss, planBord1, planBord2], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  #pipe = geompy.MakePipe(disque, WirePorteFondFissure)
  #pipe = geompy.MakePartition([pipe],[fillingFaceExterne], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  #pipes = geompy.ExtractShapes(pipe, geompy.ShapeType["SOLID"], False)
  #pipesSorted, volmin, volmax = sortSolids(pipes)
  #pipeFondFiss = pipesSorted[-1]
  #pipeFondFiss = geompy.MakePartition([pipeFondFiss], [planfiss, wireFondFiss, planBord1, planBord2], [], [], geompy.ShapeType["SOLID"], 0, [], 0)

  geomPublish(initLog.debug,  disque, 'disque')
  geomPublish(initLog.debug,  wireFondFiss, 'wireFondFiss')
  geomPublish(initLog.debug,  pipeFondFiss, 'pipeFondFiss')

  VerticesEndFondFiss = findWireEndVertices(wireFondFiss)
  for i, v in enumerate(VerticesEndFondFiss):
    name = "vertexEndFondFiss_%d"%i
    geomPublishInFather(initLog.debug,wireFondFiss, v, name)
  VerticesEndPipeFiss = []
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
  rayons = []
  demiCercles = []
  for i, edgeId in enumerate(edgesIdPipeOnside):
    if edgeId in edgesIdPipeOnplan:
      rayons.append(edgesPipeOnside[i])
    else:
      demiCercles.append(edgesPipeOnside[i])
  demiCerclesExternes = demiCercles
  rayons = rayons + edgesSorted[:4]            # les 4 plus petits sont les rayons
  demiCercles = demiCercles  + edgesSorted[4:] # les suivants sont les arcs de cercle
  rayonsId = getSubshapeIds(pipeFondFiss, rayons)
  generatrices = []
  for i, edgeId in enumerate(edgesIdPipeOnplan):
    if edgeId not in rayonsId:
      generatrices.append(edgesPipeOnplan[i])

  # --- generatrices en contact avec la face fissure externe au pipe
  generFiss = []
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
  groupsDemiCerclesPipe = []
  for i, vertex in enumerate(verticesEdgePeauFiss):
    demis = []
    for edge in demiCerclesExternes:
      if geompy.MinDistance(vertex, edge) < 0.1:
        demis.append(edge)
    group = geompy.CreateGroup(pipeFondFiss, geompy.ShapeType["EDGE"])
    geompy.UnionList(group, demis)
    name = "Cercle%d"%i
    geomPublishInFather(initLog.debug,pipeFondFiss, group , name)
    groupsDemiCerclesPipe.append(group)

  # --- faces fissure dans le pipe

  facesFissinPipe = []
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

  edgesFondFiss = []
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
  criteres = []
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

  meshFondFiss = smesh.Mesh(pipeFondFiss)
  algo2d = meshFondFiss.Quadrangle(algo=smeshBuilder.QUADRANGLE)
  algo3d = meshFondFiss.Prism()
  putName(algo3d.GetSubMesh(), "pipe")
  putName(algo3d, "algo3d_pipe")
  putName(algo2d, "algo2d_pipe")

  for i, face in enumerate(disques):
    algo2d = meshFondFiss.Quadrangle(algo=smeshBuilder.RADIAL_QUAD,geom=face)
    putName(algo2d.GetSubMesh(), "disque", i)
    putName(algo2d, "algo2d_disque", i)

  for i, edge in enumerate(rayons):
    algo1d = meshFondFiss.Segment(geom=edge)
    hypo1d = algo1d.NumberOfSegments(4)
    putName(algo1d.GetSubMesh(), "rayon", i)
    putName(algo1d, "algo1d_rayon", i)
    putName(hypo1d, "hypo1d_rayon", i)

  for i, edge in enumerate(demiCercles):
    algo1d = meshFondFiss.Segment(geom=edge)
    hypo1d = algo1d.NumberOfSegments(6)
    putName(algo1d.GetSubMesh(), "demiCercle", i)
    putName(algo1d, "algo1d_demiCercle", i)
    putName(hypo1d, "hypo1d_demiCercle", i)

  generSorted, minlg, maxlg = sortEdges(generatrices)
  nbSegGenLong = int(math.sqrt(3.0)*maxlg/(profondeur - rayonPipe)) # on veut 2 triangles equilateraux dans la largeur de la face
  nbSegGenBout = 6
  logging.info("min %s, max %s, nombre de segments %s, nombre de generatrices %s", minlg, maxlg, nbSegGenLong, len(generSorted))
  for i, edge in enumerate(generSorted):
    algo1d = meshFondFiss.Segment(geom=edge)
    if i < 6:
      hypo1d = algo1d.NumberOfSegments(nbSegGenBout)
    else:
      hypo1d = algo1d.NumberOfSegments(nbSegGenLong)
    putName(algo1d.GetSubMesh(), "generatrice", i)
    putName(algo1d, "algo1d_generatrice", i)
    putName(hypo1d, "hypo1d_generatrice", i)
  isDone = meshFondFiss.Compute()
  logging.info("meshFondFiss computed")

  disks = []
  for i, face in enumerate(disques[:4]):
    name = "disk%d"%i
    disks.append(meshFondFiss.GroupOnGeom(face, name, SMESH.FACE))
  peauext_pipe = meshFondFiss.GetMesh().UnionListOfGroups( disks, 'PEAUEXT' )

  grpPFOR = meshFondFiss.GroupOnGeom(VerticesEndPipeFiss[0], "PFOR", SMESH.NODE)
  grpPFEX = meshFondFiss.GroupOnGeom(VerticesEndPipeFiss[1], "PFEX", SMESH.NODE)

  grp = meshFondFiss.GroupOnGeom(groupFaceFissInPipe, "fisInPi", SMESH.FACE)
  group_edgeFondFiss = meshFondFiss.GroupOnGeom(groupEdgeFondFiss, "FONDFISS", SMESH.EDGE)
  noeudsFondFissure = meshFondFiss.GroupOnGeom(groupEdgeFondFiss, "nfondfis", SMESH.NODE)
  groups_demiCercles = []
  groupnodes_demiCercles = []
  for i, group in enumerate(groupsDemiCerclesPipe):
    name = "Cercle%d"%i
    groups_demiCercles.append(meshFondFiss.GroupOnGeom(group, name, SMESH.EDGE))
    name = "nCercle%d"%i
    groupnodes_demiCercles.append(meshFondFiss.GroupOnGeom(group, name, SMESH.NODE))
  group_generFiss = meshFondFiss.GroupOnGeom(groupGenerFiss, "GenFiss", SMESH.EDGE)
  groupnode_generFiss = meshFondFiss.GroupOnGeom(groupGenerFiss, "GenFiss", SMESH.NODE)
  grpNode0 = meshFondFiss.IntersectGroups(groupnode_generFiss, groupnodes_demiCercles[0], "Node0")
  grpNode1 = meshFondFiss.IntersectGroups(groupnode_generFiss, groupnodes_demiCercles[1], "Node1")
  idNode0 = grpNode0.GetID(1)
  idNode1 = grpNode1.GetID(1)
  coordsMesh = []
  coordsMesh.append(meshFondFiss.GetNodeXYZ(idNode0))
  coordsMesh.append(meshFondFiss.GetNodeXYZ(idNode1))
  coordsGeom = []
  for vertex in verticesEdgePeauFiss:
    coord = geompy.PointCoordinates(vertex);
    if distance2(coord, coordsMesh[0]) < 0.1:
      meshFondFiss.MoveNode(idNode0, coord[0], coord[1], coord[2])
    if distance2(coord, coordsMesh[1]) < 0.1:
      meshFondFiss.MoveNode(idNode1, coord[0], coord[1], coord[2])

  for groupNodes in groupnodes_demiCercles:
    for idNode in groupNodes.GetListOfID():
      coordMesh = meshFondFiss.GetNodeXYZ(idNode)
      vertex = geompy.MakeVertex(coordMesh[0], coordMesh[1], coordMesh[2])
      minDist = 100000
      minCoord = None
      imin = -1
      for i, edge in enumerate(demiCerclesPeau):
        discoord = geompy.MinDistanceComponents(vertex, edge)
        if discoord[0] <minDist:
          minDist = discoord[0]
          minCoord = discoord[1:]
          imin = i
      if imin >= 0 and minDist > 1.E-6:
        logging.debug("node id moved : %s distance=%s", idNode, minDist)
        meshFondFiss.MoveNode(idNode, coordMesh[0] + minCoord[0], coordMesh[1] + minCoord[1], coordMesh[2] + minCoord[2])


  # --- maillage face de peau

  meshFacePeau = smesh.Mesh(facePeau)
  algo2d = meshFacePeau.Triangle(algo=smeshBuilder.NETGEN_2D)
  hypo2d = algo2d.Parameters()
  hypo2d.SetMaxSize( 1000 )
  hypo2d.SetOptimize( 1 )
  hypo2d.SetFineness( 2 )
  hypo2d.SetMinSize( 2 )
  hypo2d.SetQuadAllowed( 0 )
  putName(algo2d.GetSubMesh(), "facePeau")
  putName(algo2d, "algo2d_facePeau")
  putName(hypo2d, "hypo2d_facePeau")
  #
  lenEdgePeauFiss = geompy.BasicProperties(edgePeauFiss)[0]
  frac = profondeur/lenEdgePeauFiss
  nbSeg = nbSegGenLong +2*nbSegGenBout
  ratio = (nbSegGenBout/float(profondeur)) / (nbSegGenLong/lenEdgePeauFiss)
  logging.info("lenEdgePeauFiss %s, profondeur %s, nbSegGenLong %s, nbSegGenBout %s, frac %s, ratio %s", lenEdgePeauFiss, profondeur, nbSegGenLong, nbSegGenBout, frac, ratio)
  algo1d = meshFacePeau.Segment(geom=edgePeauFiss)
  hypo1d = algo1d.NumberOfSegments(nbSeg,[],[  ])
  hypo1d.SetDistrType( 2 )
  hypo1d.SetConversionMode( 1 )
  hypo1d.SetTableFunction( [ 0, ratio, frac, 1, (1.-frac), 1, 1, ratio ] )
  putName(algo1d.GetSubMesh(), "edgePeauFiss")
  putName(algo1d, "algo1d_edgePeauFiss")
  putName(hypo1d, "hypo1d_edgePeauFiss")
  #
  algo1d = meshFacePeau.UseExisting1DElements(geom=groupEdgesBordPeau)
  hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
  putName(algo1d.GetSubMesh(), "bordsLibres")
  putName(algo1d, "algo1d_bordsLibres")
  putName(hypo1d, "hypo1d_bordsLibres")
  #
  for i in range(2):
    algo1d = meshFacePeau.UseExisting1DElements(geom=groupsDemiCerclesPeau[i])
    hypo1d = algo1d.SourceEdges([ groups_demiCercles[i] ],0,0)
    putName(algo1d.GetSubMesh(), "DemiCercles", i)
    putName(algo1d, "algo1d_groupDemiCercles", i)
    putName(hypo1d, "hypo1d_groupDemiCercles", i)
  #
  isDone = meshFacePeau.Compute()
  logging.info("meshFacePeau computed")
  grpTHOR = meshFacePeau.GroupOnGeom(verticesOutCercles[0], "THOR", SMESH.NODE)
  grpTHEX = meshFacePeau.GroupOnGeom(verticesOutCercles[1], "THEX", SMESH.NODE)

  groupEdgesPeauFiss = meshFacePeau.GroupOnGeom(edgePeauFiss, "PeauFis", SMESH.EDGE)

  peauext_face = meshFacePeau.CreateEmptyGroup( SMESH.FACE, 'PEAUEXT' )
  nbAdd = peauext_face.AddFrom( meshFacePeau.GetMesh() )


  # --- maillage face de fissure

  meshFaceFiss = smesh.Mesh(faceFiss)
  algo2d = meshFaceFiss.Triangle(algo=smeshBuilder.NETGEN_2D)
  hypo2d = algo2d.Parameters()
  hypo2d.SetMaxSize( (profondeur - rayonPipe)/math.sqrt(3.0) ) # pour avoir deux couches de triangles equilateraux partout sur la fissure
  hypo2d.SetOptimize( 1 )
  hypo2d.SetFineness( 2 )
  hypo2d.SetMinSize( 2 )
  hypo2d.SetQuadAllowed( 0 )
  putName(algo2d.GetSubMesh(), "faceFiss")
  putName(algo2d, "algo2d_faceFiss")
  putName(hypo2d, "hypo2d_faceFiss")
  #
  algo1d = meshFaceFiss.UseExisting1DElements(geom=edgePeauFiss)
  hypo1d = algo1d.SourceEdges([ groupEdgesPeauFiss ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau")
  putName(algo1d, "algo1d_edgeFissPeau")
  putName(hypo1d, "hypo1d_edgeFissPeau")
  #
  algo1d = meshFaceFiss.UseExisting1DElements(geom=groupEdgesFaceFissPipe)
  hypo1d = algo1d.SourceEdges([ group_generFiss ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau")
  putName(algo1d, "algo1d_edgeFissPeau")
  putName(hypo1d, "hypo1d_edgeFissPeau")
  #
  isDone = meshFaceFiss.Compute()
  logging.info("meshFaceFiss computed")

  grp = meshFaceFiss.GroupOnGeom(faceFiss, "fisOutPi", SMESH.FACE)

  meshBoiteDefaut = smesh.Concatenate([internalBoundary.GetMesh(),
                                   meshFondFiss.GetMesh(),
                                   meshFacePeau.GetMesh(),
                                   meshFaceFiss.GetMesh()],
                                   1, 1, 1e-05,False)
  # pour aider l'algo hexa-tetra a ne pas mettre de pyramides a l'exterieur des volumes replies sur eux-memes
  # on designe les faces de peau en quadrangles par le groupe "skinFaces"
  group_faceFissOutPipe = None
  group_faceFissInPipe = None
  groups = meshBoiteDefaut.GetGroups()
  for grp in groups:
    if grp.GetType() == SMESH.FACE:
      #if "internalBoundary" in grp.GetName():
      #  grp.SetName("skinFaces")
      if grp.GetName() == "fisOutPi":
        group_faceFissOutPipe = grp
      elif grp.GetName() == "fisInPi":
        group_faceFissInPipe = grp

  # le maillage NETGEN ne passe pas toujours ==> utiliser GHS3D
  distene=True
  if distene:
    algo3d = meshBoiteDefaut.Tetrahedron(algo=smeshBuilder.GHS3D)
  else:
    algo3d = meshBoiteDefaut.Tetrahedron(algo=smeshBuilder.NETGEN)
    hypo3d = algo3d.MaxElementVolume(1000.0)
  putName(algo3d.GetSubMesh(), "boiteDefaut")
  putName(algo3d, "algo3d_boiteDefaut")
  isDone = meshBoiteDefaut.Compute()
  logging.info("meshBoiteDefaut computed")
  putName(meshBoiteDefaut, "boiteDefaut")

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

  maillageComplet.ExportMED( fichierMaillageFissure, 0, SMESH.MED_V2_2, 1 )
  putName(maillageComplet, nomFicFissure)
  logging.info("fichier maillage fissure %s", fichierMaillageFissure)

  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(True)

  return  maillageComplet