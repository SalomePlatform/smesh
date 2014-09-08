# -*- coding: utf-8 -*-

import logging
import salome
from geomsmesh import geompy
import GEOM
from geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH
import math
import bisect

from extractionOrientee import extractionOrientee
from extractionOrienteeMulti import extractionOrienteeMulti
from sortFaces import sortFaces
from sortEdges import sortEdges
from eliminateDoubles import eliminateDoubles
from substractSubShapes import substractSubShapes
from produitMixte import produitMixte
from findWireEndVertices import findWireEndVertices
from findWireIntermediateVertices import findWireIntermediateVertices
from orderEdgesFromWire import orderEdgesFromWire
from getSubshapeIds import getSubshapeIds
from putName import putName
from distance2 import distance2
from enleveDefaut import enleveDefaut
from shapeSurFissure import shapeSurFissure
from regroupeSainEtDefaut import RegroupeSainEtDefaut
from triedreBase import triedreBase
from checkDecoupePartition import checkDecoupePartition
from whichSide import whichSide
from whichSideMulti import whichSideMulti
from whichSideVertex import whichSideVertex
from projettePointSurCourbe import projettePointSurCourbe
from prolongeWire import prolongeWire
from restreintFaceFissure import restreintFaceFissure
from partitionneFissureParPipe import partitionneFissureParPipe
from construitPartitionsPeauFissure import construitPartitionsPeauFissure
from compoundFromList import compoundFromList
from identifieElementsGeometriquesPeau import identifieElementsGeometriquesPeau

# -----------------------------------------------------------------------------
# --- procédure complète fissure générale

def construitFissureGenerale(maillagesSains,
                             shapesFissure, shapeFissureParams,
                             maillageFissureParams, elementsDefaut, step=-1):
  """
  TODO: a completer
  """
  logging.info('start')
  
  shapeDefaut       = shapesFissure[0] # faces de fissure, débordant
  fondFiss          = shapesFissure[4] # groupe d'edges de fond de fissure

  rayonPipe = shapeFissureParams['rayonPipe']
  if shapeFissureParams.has_key('lenSegPipe'):
    lenSegPipe = shapeFissureParams['lenSegPipe']
  else:
    lenSegPipe = rayonPipe

  nomRep            = maillageFissureParams['nomRep']
  nomFicSain        = maillageFissureParams['nomFicSain']
  nomFicFissure     = maillageFissureParams['nomFicFissure']

  nbsegRad          = maillageFissureParams['nbsegRad']      # nombre de couches selon un rayon du pipe
  nbsegCercle       = maillageFissureParams['nbsegCercle']   # nombre de secteur dans un cercle du pipe
  areteFaceFissure  = maillageFissureParams['areteFaceFissure']

  pointIn_x = 0.0
  pointIn_y = 0.0
  pointIn_z = 0.0
  isPointInterne = False
  if shapeFissureParams.has_key('pointIn_x'):
    pointIn_x = shapeFissureParams['pointIn_x']
    isPointInterne = True
  if shapeFissureParams.has_key('pointIn_y'):
    pointIn_y = shapeFissureParams['pointIn_y']
    isPointInterne = True
  if shapeFissureParams.has_key('pointIn_z'):
    pointIn_z = shapeFissureParams['pointIn_z']
    isPointInterne = True
  if isPointInterne:
    pointInterne = geompy.MakeVertex(pointIn_x, pointIn_y, pointIn_z)
  else:
    pointInterne = None

  #fichierMaillageSain = nomRep + '/' + nomFicSain + '.med'
  fichierMaillageFissure = nomRep + '/' + nomFicFissure + '.med'

  # fillings des faces en peau
  facesDefaut = elementsDefaut[0]
  #centresDefaut            = elementsDefaut[1]
  #normalsDefaut            = elementsDefaut[2]
  #extrusionsDefaut         = elementsDefaut[3]
  dmoyen                   = elementsDefaut[4]
  bordsPartages = elementsDefaut[5]
  fillconts                = elementsDefaut[6]
  idFilToCont              = elementsDefaut[7]
  maillageSain             = elementsDefaut[8]
  internalBoundary         = elementsDefaut[9]
  zoneDefaut               = elementsDefaut[10]
  zoneDefaut_skin          = elementsDefaut[11]
  zoneDefaut_internalFaces = elementsDefaut[12]
  zoneDefaut_internalEdges = elementsDefaut[13]
  edgeFondExt              = elementsDefaut[14]
  centreFondFiss           = elementsDefaut[15]
  tgtCentre                = elementsDefaut[16]

  O, OX, OY, OZ = triedreBase()

  # --- restriction de la face de fissure au domaine solide :
  #     partition face fissure étendue par fillings, on garde la face interne
  
  facesPortFissure = restreintFaceFissure(shapeDefaut, facesDefaut, pointInterne)
 
  # --- pipe de fond de fissure, prolongé, partition face fissure par pipe
  #     identification des edges communes pipe et face fissure
  
  (fissPipe, edgesPipeFiss, edgesFondFiss, wirePipeFiss, wireFondFiss) = partitionneFissureParPipe(shapesFissure, elementsDefaut, rayonPipe)
  edgesFondFiss, edgesIdByOrientation = orderEdgesFromWire(wireFondFiss)
  for i,edge in enumerate(edgesFondFiss):
    geompy.addToStudyInFather(wireFondFiss, edge, "edgeFondFiss%d"%i)
  
  # --- peau et face de fissure
  #
  # --- partition peau défaut - face de fissure prolongée - wire de fond de fissure prolongée
  #     il peut y avoir plusieurs faces externes, dont certaines sont découpées par la fissure
  #     liste de faces externes : facesDefaut
  #     liste de partitions face externe - fissure : partitionPeauFissFond (None quand pas d'intersection)

  partitionsPeauFissFond = construitPartitionsPeauFissure(facesDefaut, fissPipe)
    
  # --- arêtes vives détectées (dans quadranglesToShapeNoCorner
  #                             et quadranglesToShapeWithCorner)
    
  aretesVivesCoupees = []  #= global
  
  aretesVivesC = compoundFromList(bordsPartages, "areteVive")
 
  # -------------------------------------------------------
  # --- inventaire des faces de peau coupées par la fissure
  #     pour chaque face de peau : 0, 1 ou 2 faces débouchante du fond de fissure
  #                                0, 1 ou plus edges de la face de fissure externe au pipe
  
  nbFacesFilling = len(partitionsPeauFissFond)
  
  ptEdgeFond = [ []  for i in range(nbFacesFilling)] # pour chaque face [points edge fond de fissure aux débouchés du pipe]
  fsPipePeau = [ []  for i in range(nbFacesFilling)] # pour chaque face [faces du pipe débouchantes]
  edRadFPiPo = [ []  for i in range(nbFacesFilling)] # pour chaque face [edge radiale des faces du pipe débouchantes ]
  fsFissuExt = [ []  for i in range(nbFacesFilling)] # pour chaque face [faces de fissure externes au pipe]
  edFisExtPe = [ []  for i in range(nbFacesFilling)] # pour chaque face [edge en peau des faces de fissure externes (pas subshape facePeau)]
  edFisExtPi = [ []  for i in range(nbFacesFilling)] # pour chaque face [edge commun au pipe des faces de fissure externes]
  facesPeaux = [None for i in range(nbFacesFilling)] # pour chaque face : la face de peau finale a mailler (percée des faces débouchantes)
  edCircPeau = [ []  for i in range(nbFacesFilling)] # pour chaque face de peau : [subshape edge circulaire aux débouchés du pipe]
  ptCircPeau = [ []  for i in range(nbFacesFilling)] # pour chaque face de peau : [subshape point sur edge circulaire aux débouchés du pipe]
  gpedgeBord = [None for i in range(nbFacesFilling)] # pour chaque face de peau : groupe subshape des edges aux bords liés à la partie saine
  gpedgeVifs = [None for i in range(nbFacesFilling)] # pour chaque face de peau : groupes subshape des edges aux arêtes vives entre fillings
  edFissPeau = [ []  for i in range(nbFacesFilling)] # pour chaque face de peau : [subshape edge en peau des faces de fissure externes]
  ptFisExtPi = [ []  for i in range(nbFacesFilling)] # pour chaque face de peau : [point commun edFissPeau edCircPeau]
  
  for ifil, partitionPeauFissFond in enumerate(partitionsPeauFissFond):
    if partitionPeauFissFond is not None:
      dataPPFF,aretesVivesCoupees = identifieElementsGeometriquesPeau(ifil, partitionPeauFissFond, edgesPipeFiss,
                                                                      edgesFondFiss, wireFondFiss, aretesVivesC,
                                                                      facesDefaut, centreFondFiss, rayonPipe,
                                                                      aretesVivesCoupees)
      
      ptEdgeFond[ifil] = dataPPFF['endsEdgeFond']
      fsPipePeau[ifil] = dataPPFF['facesPipePeau']
      edRadFPiPo[ifil] = dataPPFF['edgeRadFacePipePeau']
      fsFissuExt[ifil] = dataPPFF['facesFissExt']
      edFisExtPe[ifil] = dataPPFF['edgesFissExtPeau']
      edFisExtPi[ifil] = dataPPFF['edgesFissExtPipe']
      facesPeaux[ifil] = dataPPFF['facePeau']
      edCircPeau[ifil] = dataPPFF['edgesCircPeau']
      ptCircPeau[ifil] = dataPPFF['verticesCircPeau']
      gpedgeBord[ifil] = dataPPFF['groupEdgesBordPeau']
      gpedgeVifs[ifil] = dataPPFF['bordsVifs']
      edFissPeau[ifil] = dataPPFF['edgesFissurePeau']
      ptFisExtPi[ifil] = dataPPFF['verticesPipePeau']

  # -----------------------------------------------------------------------
  # fin de la boucle sur les faces de filling
  # -----------------------------------------------------------------------
  
  for i, avc in enumerate(aretesVivesCoupees):
    name = "areteViveCoupee%d"%i
    geompy.addToStudy(avc, name)
  
  # --- identification des faces et edges de fissure externe pour maillage
  
  facesFissExt = []
  edgesFissExtPeau = []
  edgesFissExtPipe = []
  for ifil in range(nbFacesFilling): # TODO: éliminer les doublons (comparer tous les vertices triés, avec mesure de distance ?)
    facesFissExt += fsFissuExt[ifil]
    edgesFissExtPeau += edFisExtPe[ifil]
    edgesFissExtPipe += edFisExtPi[ifil]
  logging.debug("---------------------------- identification faces de fissure externes au pipe :%s ", len(facesFissExt))
  # regroupement des faces de fissure externes au pipe.
  
  if len(facesFissExt) > 1:
    faceFissureExterne = geompy.MakePartition(facesFissExt, [], [], [], geompy.ShapeType["FACE"], 0, [], 0)
    edgesPipeFissureExterneC = geompy.GetInPlace(faceFissureExterne, geompy.MakeCompound(edgesPipeFiss))    # edgesFissExtPipe peut ne pas couvrir toute la longueur
    # edgesPeauFissureExterneC = geompy.GetInPlace(faceFissureExterne, geompy.MakeCompound(edgesFissExtPeau))
    # il peut manquer des edges de faceFissureExterne en contact avec la peau dans edgesFissExtPeau
    (isDone, closedFreeBoundaries, openFreeBoundaries) = geompy.GetFreeBoundary(faceFissureExterne)
    edgesBordFFE = []
    for bound in closedFreeBoundaries:
      edgesBordFFE += geompy.ExtractShapes(bound, geompy.ShapeType["EDGE"], False)
    edgesBordFFEid = [ (ed,geompy.GetSubShapeID(faceFissureExterne, ed)) for ed in edgesBordFFE]
    logging.debug("edgesBordFFEid %s", edgesBordFFEid)
    edgesPPE = geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False)
    edgesPPEid = [ geompy.GetSubShapeID(faceFissureExterne, ed) for ed in edgesPPE]
    logging.debug("edgesPPEid %s", edgesPPEid)
    edgesPFE = [ edid[0] for edid in edgesBordFFEid if edid[1] not in edgesPPEid] # on garde toutes les edges de bord non en contact avec le pipe
    logging.debug("edgesPFE %s", edgesPFE)
    edgesPeauFissureExterneC = geompy.MakeCompound(edgesPFE)
  else:
    faceFissureExterne = facesFissExt[0]
    edgesPeauFissureExterneC = geompy.MakeCompound(edgesFissExtPeau)
    edgesPipeFissureExterneC = geompy.MakeCompound(edgesFissExtPipe)
  wirePipeFissureExterne = geompy.MakeWire(geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False))
  geompy.addToStudy(faceFissureExterne, "faceFissureExterne")
  geompy.addToStudyInFather(faceFissureExterne, edgesPeauFissureExterneC, "edgesPeauFissureExterne")
  geompy.addToStudyInFather(faceFissureExterne, edgesPipeFissureExterneC, "edgesPipeFissureExterne")
  
  logging.debug("---------------------------- Preparation Maillage du Pipe --------------")
  # -----------------------------------------------------------------------
  # --- preparation maillage du pipe :
  #     - détections des points a respecter : jonction des edges/faces constituant
  #       la face de fissure externe au pipe
  #     - points sur les edges de fond de fissure et edges pipe/face fissure,
  #     - vecteurs tangents au fond de fissure (normal au disque maillé)

  # --- option de maillage selon le rayon de courbure du fond de fissure 
  lenEdgeFondExt = 0
  for edff in edgesFondFiss:
    lenEdgeFondExt += geompy.BasicProperties(edff)[0]
  
  disfond = []
  for filling in facesDefaut:
    disfond.append(geompy.MinDistance(centreFondFiss, filling))
  disfond.sort()
  rcourb = disfond[0]
  nbSegQuart = 5 # on veut 5 segments min sur un quart de cercle
  alpha = math.pi/(4*nbSegQuart)
  deflexion = rcourb*(1.0 -math.cos(alpha))
  lgmin = lenSegPipe*0.25
  lgmax = lenSegPipe*1.5               
  logging.debug("rcourb: %s, lenFond:%s, deflexion: %s, lgmin: %s, lgmax: %s", rcourb, lenEdgeFondExt, deflexion, lgmin, lgmax)  

  meshFondExt = smesh.Mesh(wireFondFiss)
  algo1d = meshFondExt.Segment()
  hypo1d = algo1d.Adaptive(lgmin, lgmax, deflexion) # a ajuster selon la profondeur de la fissure
  isDone = meshFondExt.Compute()
  
  ptGSdic = {} # dictionnaire [paramètre sur la courbe] --> point géométrique
  allNodeIds = meshFondExt.GetNodesId()
  for nodeId in allNodeIds:
    xyz = meshFondExt.GetNodeXYZ(nodeId)
    #logging.debug("nodeId %s, coords %s", nodeId, str(xyz))
    pt = geompy.MakeVertex(xyz[0], xyz[1], xyz[2])
    u, PointOnEdge, EdgeInWireIndex = geompy.MakeProjectionOnWire(pt, wireFondFiss) # u compris entre 0 et 1
    edgeOrder = edgesIdByOrientation[EdgeInWireIndex]
    ptGSdic[(edgeOrder, EdgeInWireIndex, u)] = pt
    #logging.debug("nodeId %s, u %s", nodeId, str(u))
  usort = sorted(ptGSdic)  
  logging.debug("nombre de points obtenus par deflexion %s",len(usort))
     
  centres = []
  origins = []
  normals = []      
  for edu in usort:
    ied = edu[1]
    u = edu[2]
    vertcx = ptGSdic[edu]
    norm = geompy.MakeTangentOnCurve(edgesFondFiss[ied], u)
    plan = geompy.MakePlane(vertcx, norm, 3*rayonPipe)
    part = geompy.MakePartition([plan], [wirePipeFiss], [], [], geompy.ShapeType["VERTEX"], 0, [], 0)
    liste = geompy.ExtractShapes(part, geompy.ShapeType["VERTEX"], True)
    if len(liste) == 5: # 4 coins du plan plus intersection recherchée
      for point in liste:
        if geompy.MinDistance(point, vertcx) < 1.1*rayonPipe: # les quatre coins sont plus loin
          vertpx = point
          break
      centres.append(vertcx)
      origins.append(vertpx)
      normals.append(norm)
#      name = "vertcx%d"%i
#      geompy.addToStudyInFather(wireFondFiss, vertcx, name)
#      name = "vertpx%d"%i
#      geompy.addToStudyInFather(wireFondFiss, vertpx, name)
#      name = "plan%d"%i
#      geompy.addToStudyInFather(wireFondFiss, plan, name)

  # --- maillage du pipe étendu, sans tenir compte de l'intersection avec la face de peau
      
  logging.debug("nbsegCercle %s", nbsegCercle)
  
  # -----------------------------------------------------------------------
  # --- points géométriques
  
  gptsdisks = [] # vertices géométrie de tous les disques
  raydisks = [[] for i in range(nbsegCercle)]
  for i in range(len(centres)): # boucle sur les disques
    gptdsk = [] # vertices géométrie d'un disque
    vertcx = centres[i]
    vertpx = origins[i]
    normal = normals[i]
    vec1 = geompy.MakeVector(vertcx, vertpx)
    
    points = [vertcx] # les points du rayon de référence
    for j in range(nbsegRad):
      pt = geompy.MakeTranslationVectorDistance(vertcx, vec1, (j+1)*float(rayonPipe)/nbsegRad)
      points.append(pt)
    gptdsk.append(points)
    pt = geompy.MakeTranslationVectorDistance(vertcx, vec1, 1.5*rayonPipe)
    rayon = geompy.MakeLineTwoPnt(vertcx, pt)
    raydisks[0].append(rayon)
    
    for k in range(nbsegCercle-1):
      angle = (k+1)*2*math.pi/nbsegCercle
      pts = [vertcx] # les points d'un rayon obtenu par rotation
      for j in range(nbsegRad):
        pt = geompy.MakeRotation(points[j+1], normal, angle)
        pts.append(pt)
      gptdsk.append(pts)
      ray = geompy.MakeRotation(rayon, normal, angle)
      raydisks[k+1].append(ray)
      
    gptsdisks.append(gptdsk) 
 
  # -----------------------------------------------------------------------
  # --- recherche des points en trop (externes au volume à remailler)
  #     - on associe chaque extrémité du pipe à une face filling 
  #     - on part des disques aux extrémités du pipe
  #     - pour chaque disque, on prend les vertices de géométrie,
  #       on marque leur position relative à la face.
  #     - on s'arrete quand tous les noeuds sont dedans
  
  logging.debug("---------------------------- recherche des points du pipe a éliminer --------------")

  pt0 = centres[0]
  pt1 = centres[-1]
  idFillingFromBout = [None, None]                 # contiendra l'index du filling pour les extrémités 0 et 1
  for ifil in range(nbFacesFilling):
    for ipt, pt in enumerate(ptEdgeFond[ifil]): # il y a un ou deux points débouchant sur cette face
      if geompy.MinDistance(pt,pt0) < geompy.MinDistance(pt,pt1): # TODO: trouver plus fiable pour les cas tordus...
        idFillingFromBout[0] = ifil
      else: 
        idFillingFromBout[1] = ifil
  logging.debug("association bouts du pipe - faces de filling: %s", idFillingFromBout)
  
  facesPipePeau = []
  edgeRadFacePipePeau = []
  for ifil in range(nbFacesFilling):
    facesPipePeau += fsPipePeau[ifil]
    edgeRadFacePipePeau += edRadFPiPo[ifil]
    
  logging.debug("recherche des disques de noeuds complètement internes")
  idisklim = [] # indices des premier et dernier disques internes
  idiskout = [] # indices des premier et dernier disques externes
  for bout in range(2):
    if bout == 0:
      idisk = -1
      inc = 1
      numout = -1
    else:
      idisk = len(gptsdisks)
      inc = -1
      numout = len(gptsdisks)
    inside = False
    outside = True
    while not inside:
      idisk = idisk + inc
      logging.debug("examen disque %s", idisk)
      gptdsk = gptsdisks[idisk]
      inside = True
      for k in range(nbsegCercle):
        points = gptdsk[k]
        for j, pt in enumerate(points):
          side = whichSideVertex(facesDefaut[idFillingFromBout[bout]], pt)
          if side < 0:
            if outside: # premier point detecté dedans
              outside = False
              numout = idisk -inc # le disque précédent était dehors
          else:
            inside = False # ce point est dehors              
        if not inside and not outside:
          break
    idisklim.append(idisk)  # premier et dernier disques internes
    idiskout.append(numout) # premier et dernier disques externes
    
  # --- listes de nappes radiales en filling à chaque extrémité débouchante
  facesDebouchantes = [False, False]
  idFacesDebouchantes = [-1, -1] # contiendra les indices des faces disque débouchantes (facesPipePeau)
  listNappes =[]
  for i, idisk in enumerate(idisklim):
    numout = idiskout[i]
    logging.debug("extremité %s, indices disques interne %s, externe %s",i, idisk, numout)
    nappes = []
    if  (idisk != 0) and (idisk != len(gptsdisks)-1): # si extrémité débouchante
      for k in range(nbsegCercle):
        if i == 0:
          iddeb = max(0, numout)
          idfin = max(iddeb+3,idisk+1) # il faut 3 rayons pour faire un filling qui suive le fond de fissure
          #logging.debug("extremité %s, indices retenus interne %s, externe %s",i, idfin, iddeb)
          comp = geompy.MakeCompound(raydisks[k][iddeb:idfin])
          name='compoundRay%d'%k
          geompy.addToStudy(comp, name)
        else:
          idfin = min(len(gptsdisks), numout+1)
          iddeb = min(idfin-3, idisk) # il faut 3 rayons pour faire un filling qui suive le fond de fissure
          #logging.debug("extremité %s, indices retenus interne %s, externe %s",i, idfin, iddeb)
          comp = geompy.MakeCompound(raydisks[k][iddeb:idfin])
          name='compoundRay%d'%k
          geompy.addToStudy(comp, name)
        nappe = geompy.MakeFilling(comp, 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default)
        nappes.append(nappe)
        name='nappe%d'%k
        geompy.addToStudy(nappe, name)
        facesDebouchantes[i] = True
    listNappes.append(nappes)
      
  # --- mise en correspondance avec les indices des faces disque débouchantes (facesPipePeau)
  for i, nappes in enumerate(listNappes):
    if facesDebouchantes[i]:
      for k, face in enumerate(facesPipePeau):
        edge = geompy.MakeSection(face, nappes[0])
        if geompy.NbShapes(edge, geompy.ShapeType["EDGE"]) > 0:
          idFacesDebouchantes[i] = k
          break
  logging.debug("idFacesDebouchantes: %s", idFacesDebouchantes)

  # --- construction des listes d'edges radiales sur chaque extrémité débouchante
  listEdges = []
  for i, nappes in enumerate(listNappes):
    id = idFacesDebouchantes[i] # indice de face débouchante (facesPipePeau)
    if id < 0:
      listEdges.append([])
    else:
      face = facesPipePeau[id]
      edges = [edgeRadFacePipePeau[id]]
      for k, nappe in enumerate(nappes):
        if k > 0:
          obj = geompy.MakeSection(face, nappes[k]) # normalement une edge, parfois un compound d'edges dont un tout petit
          edge = obj
          vs = geompy.ExtractShapes(obj, geompy.ShapeType["VERTEX"], False)
          if len(vs) > 2:
            eds = geompy.ExtractShapes(obj, geompy.ShapeType["EDGE"], False)
            [edsorted, minl,maxl] = sortEdges(eds)
            edge = edsorted[-1]
          else:
            maxl = geompy.BasicProperties(edge)[0]
          if maxl < 0.01: # problème MakeSection
            logging.debug("problème MakeSection recherche edge radiale %s, longueur trop faible: %s, utilisation partition", k, maxl)
            partNappeFace = geompy.MakePartition([face, nappes[k]], [] , [], [], geompy.ShapeType["FACE"], 0, [], 0)
            edps= geompy.ExtractShapes(partNappeFace, geompy.ShapeType["EDGE"], False)
            ednouv = []
            for ii, ed in enumerate(edps):
              vxs = geompy.ExtractShapes(ed, geompy.ShapeType["VERTEX"], False)
              distx = [geompy.MinDistance(vx, face) for vx in vxs]
              distx += [geompy.MinDistance(vx, nappes[k]) for vx in vxs]
              dmax = max(distx)
              logging.debug("  dmax %s",dmax)
              if dmax < 0.01:
                ednouv.append(ed)
            logging.debug("  edges issues de la partition: %s", ednouv)
            for ii, ed in enumerate(ednouv):
              geompy.addToStudy(ed, "ednouv%d"%ii)
            [edsorted, minl,maxl] = sortEdges(ednouv)
            logging.debug("  longueur edge trouvée: %s", maxl) 
            edge = edsorted[-1]
          edges.append(edge)
          name = 'edgeEndPipe%d'%k
          geompy.addToStudy(edge, name)
      listEdges.append(edges)

  # --- création des points du maillage du pipe sur la face de peau
  for i, edges in enumerate(listEdges):
    id = idFacesDebouchantes[i] # indice de face débouchante (facesPipePeau)
    if id >= 0:
      gptdsk = []
      if id > 0: # id vaut 0 ou 1
        id = -1  # si id vaut 1, on prend le dernier élément de la liste (1 ou 2 extrémités débouchent sur la face)
      centre = ptEdgeFond[idFillingFromBout[i]][id]
      name = "centre%d"%id
      geompy.addToStudy(centre, name)
      vertPipePeau = ptFisExtPi[idFillingFromBout[i]][id]
      geompy.addToStudyInFather(centre, vertPipePeau, "vertPipePeau")
      grpsEdgesCirc = edCircPeau[idFillingFromBout[i]] # liste de groupes
      edgesCirc = []
      for grpEdgesCirc in grpsEdgesCirc:
        edgesCirc += geompy.ExtractShapes(grpEdgesCirc, geompy.ShapeType["EDGE"], False)
      for k, edge in enumerate(edges):
        extrems = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], True)
        if geompy.MinDistance(centre, extrems[0]) < geompy.MinDistance(centre, extrems[1]):
          bout = extrems[1]
        else:
          bout = extrems[0]
        # ajustement du point extrémité (bout) sur l'edge circulaire en face de peau
        logging.debug("edgesCirc: %s", edgesCirc)
        distEdgeCirc = [(geompy.MinDistance(bout, edgeCirc), k2, edgeCirc) for k2, edgeCirc in enumerate(edgesCirc)]
        distEdgeCirc.sort()
        logging.debug("distEdgeCirc: %s", distEdgeCirc)
        u = projettePointSurCourbe(bout, distEdgeCirc[0][2])
        if (abs(u) < 0.02) or (abs(1-u) < 0.02): # les points très proches d'une extrémité doivent y être mis précisément.
          extrCircs = geompy.ExtractShapes(distEdgeCirc[0][2], geompy.ShapeType["VERTEX"], True)
          if geompy.MinDistance(bout, extrCircs[0]) < geompy.MinDistance(bout, extrCircs[1]):
            bout = extrCircs[0]
          else:
            bout = extrCircs[1]
          pass
        else:
          bout = geompy.MakeVertexOnCurve(distEdgeCirc[0][2], u)
        name ="bout%d"%k
        geompy.addToStudyInFather(centre, bout, name)
        # enregistrement des points dans la structure
        points = []
        for j in range(nbsegRad +1):
          u = j/float(nbsegRad)
          points.append(geompy.MakeVertexOnCurve(edge, u))
        if geompy.MinDistance(bout, points[0]) < geompy.MinDistance(centre, points[0]):
          points.reverse()
        points[0] = centre
        points[-1] = bout
        gptdsk.append(points)
      if i == 0:
        gptsdisks[idisklim[0] -1] = gptdsk
        idisklim[0] = idisklim[0] -1
      else:
        gptsdisks[idisklim[1] +1] = gptdsk
        idisklim[1] = idisklim[1] +1

  # --- ajustement precis des points sur edgesPipeFissureExterneC
  
  edgesPFE = geompy.ExtractShapes(edgesPipeFissureExterneC, geompy.ShapeType["EDGE"], False)
  verticesPFE = findWireIntermediateVertices(wirePipeFissureExterne)  # vertices intermédiaires (des points en trop dans ptsInWireFissExtPipe)
  idiskmin = idisklim[0] + 1 # on ne prend pas le disque sur la peau, déjà ajusté
  idiskmax = idisklim[1]     # on ne prend pas le disque sur la peau, déjà ajusté
  idiskint = []
  for vtx in verticesPFE:
    distPtVt = []
    for idisk in range(idiskmin, idiskmax):
      gptdsk = gptsdisks[idisk]
      pt = gptdsk[0][-1]       # le point sur l'edge de la fissure externe au pipe
      distPtVt.append((geompy.MinDistance(pt, vtx), idisk))
    distPtVt.sort()
    idiskint.append(distPtVt[0][1])
    gptsdisks[idiskint[-1]][0][-1] = vtx
    logging.debug("ajustement point sur edgePipeFissureExterne, vertex: %s %s", idiskint[-1], distPtVt[0][0])
  for idisk in range(idiskmin, idiskmax):
    if idisk in idiskint:
      break
    logging.debug("ajustement point sur edgePipeFissureExterne: %s", idisk)
    gptdsk = gptsdisks[idisk]
    pt = gptdsk[0][-1]       # le point sur l'edge de la fissure externe au pipe
    distPtEd = [(geompy.MinDistance(pt, edgePFE), k, edgePFE) for k, edgePFE in enumerate(edgesPFE)]
    distPtEd.sort()
    edgePFE = distPtEd[0][2]
    u = projettePointSurCourbe(pt, edgePFE)
    ptproj = geompy.MakeVertexOnCurve(edgePFE, u)
    gptsdisks[idisk][0][-1] = ptproj
  
  # -----------------------------------------------------------------------
  # --- maillage effectif du pipe

  logging.debug("---------------------------- maillage effectif du pipe --------------")
  meshPipe = smesh.Mesh(None, "meshPipe")
  fondFissGroup = meshPipe.CreateEmptyGroup(SMESH.EDGE, "FONDFISS")
  nodesFondFissGroup = meshPipe.CreateEmptyGroup(SMESH.NODE, "nfondfis")
  faceFissGroup = meshPipe.CreateEmptyGroup(SMESH.FACE, "fisInPi")
  edgeFaceFissGroup = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeFaceFiss")
  edgeCircPipe0Group = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeCircPipe0")
  edgeCircPipe1Group = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeCircPipe1")
  faceCircPipe0Group = meshPipe.CreateEmptyGroup(SMESH.FACE, "faceCircPipe0")
  faceCircPipe1Group = meshPipe.CreateEmptyGroup(SMESH.FACE, "faceCircPipe1")
  mptsdisks = []  # vertices maillage de tous les disques
  mEdges = []     # identifiants edges maillage fond de fissure
  mEdgeFaces = [] # identifiants edges maillage edge face de fissure externe
  mFaces = []     # identifiants faces maillage fissure
  mVols  = []     # identifiants volumes maillage pipe

  mptdsk = None
  for idisk in range(idisklim[0], idisklim[1]+1): # boucle sur les disques internes
    
    # -----------------------------------------------------------------------
    # --- points
    
    gptdsk = gptsdisks[idisk]
    if idisk > idisklim[0]:
      oldmpts = mptdsk
    mptdsk = [] # vertices maillage d'un disque
    for k in range(nbsegCercle):
      points = gptdsk[k]
      mptids = []
      for j, pt in enumerate(points):
        if j == 0 and k > 0:
          id = mptdsk[0][0]
        else:
          coords = geompy.PointCoordinates(pt)
          id = meshPipe.AddNode(coords[0], coords[1], coords[2])
        mptids.append(id)
      mptdsk.append(mptids)
    mptsdisks.append(mptdsk)
    
    # -----------------------------------------------------------------------
    # --- groupes edges cercles debouchants
    
    if idisk == idisklim[0]:
      pts = []
      for k in range(nbsegCercle):
        pts.append(mptdsk[k][-1])
      edges = []
      for k in range(len(pts)):
        k1 = (k+1)%len(pts)
        idEdge = meshPipe.AddEdge([pts[k], pts[k1]])
        edges.append(idEdge)
      edgeCircPipe0Group.Add(edges)
       
    if idisk == idisklim[1]:
      pts = []
      for k in range(nbsegCercle):
        pts.append(mptdsk[k][-1])
      edges = []
      for k in range(len(pts)):
        k1 = (k+1)%len(pts)
        idEdge = meshPipe.AddEdge([pts[k], pts[k1]])
        edges.append(idEdge)
      edgeCircPipe1Group.Add(edges)
    
    # -----------------------------------------------------------------------
    # --- groupes faces  debouchantes
    
    if idisk == idisklim[0]:
      faces = []
      for j in range(nbsegRad):
        for k in range(nbsegCercle):
          k1 = k+1
          if k ==  nbsegCercle-1:
            k1 = 0
          if j == 0:
            idf = meshPipe.AddFace([mptdsk[k][0], mptdsk[k][1], mptdsk[k1][1]]) # triangle
          else:
            idf = meshPipe.AddFace([mptdsk[k][j], mptdsk[k][j+1], mptdsk[k1][j+1], mptdsk[k1][j]]) # quadrangle
          faces.append(idf)
      faceCircPipe0Group.Add(faces)

    if idisk == idisklim[1]:
      faces = []
      for j in range(nbsegRad):
        for k in range(nbsegCercle):
          k1 = k+1
          if k ==  nbsegCercle-1:
            k1 = 0
          if j == 0:
            idf = meshPipe.AddFace([mptdsk[k][0], mptdsk[k][1], mptdsk[k1][1]]) # triangle
          else:
            idf = meshPipe.AddFace([mptdsk[k][j], mptdsk[k][j+1], mptdsk[k1][j+1], mptdsk[k1][j]]) # quadrangle
          faces.append(idf)
      faceCircPipe1Group.Add(faces)
          
    # -----------------------------------------------------------------------
    # --- mailles volumiques, groupes noeuds et edges de fond de fissure, groupe de face de fissure
    
    if idisk == idisklim[0]:
      mEdges.append(0)
      mEdgeFaces.append(0)
      mFaces.append([0])
      mVols.append([[0]])
      nodesFondFissGroup.Add([mptdsk[0][0]])
    else:
      ide = meshPipe.AddEdge([oldmpts[0][0], mptdsk[0][0]])
      mEdges.append(ide)
      fondFissGroup.Add([ide])
      nodesFondFissGroup.Add([mptdsk[0][0]])
      ide2 = meshPipe.AddEdge([oldmpts[0][-1], mptdsk[0][-1]])
      mEdgeFaces.append(ide2)
      edgeFaceFissGroup.Add([ide2])
      idFaces = []
      idVols = []
      
      for j in range(nbsegRad):
        idf = meshPipe.AddFace([oldmpts[0][j], mptdsk[0][j], mptdsk[0][j+1], oldmpts[0][j+1]])
        faceFissGroup.Add([idf])
        idFaces.append(idf)
        
        idVolCercle = []
        for k in range(nbsegCercle):
          k1 = k+1
          if k ==  nbsegCercle-1:
            k1 = 0
          if j == 0:
            idv = meshPipe.AddVolume([mptdsk[k][j], mptdsk[k][j+1], mptdsk[k1][j+1],
                                      oldmpts[k][j], oldmpts[k][j+1], oldmpts[k1][j+1]])
          else:
            idv = meshPipe.AddVolume([mptdsk[k][j], mptdsk[k][j+1], mptdsk[k1][j+1], mptdsk[k1][j],
                                      oldmpts[k][j], oldmpts[k][j+1], oldmpts[k1][j+1], oldmpts[k1][j]])
          idVolCercle.append(idv)
        idVols.append(idVolCercle)  
        
      mFaces.append(idFaces)
      mVols.append(idVols)

  pipeFissGroup = meshPipe.CreateEmptyGroup( SMESH.VOLUME, 'PIPEFISS' )
  nbAdd = pipeFissGroup.AddFrom( meshPipe.GetMesh() )

  nb, new_mesh, new_group = meshPipe.MakeBoundaryElements(SMESH.BND_2DFROM3D, "pipeBoundaries")
  edgesCircPipeGroup = [edgeCircPipe0Group, edgeCircPipe1Group]

  # --- fin du maillage du pipe
  # -----------------------------------------------------------------------
  # --- edges de bord, faces défaut à respecter

  aFilterManager = smesh.CreateFilterManager()
  nbAdded, internalBoundary, _NoneGroup = internalBoundary.MakeBoundaryElements( SMESH.BND_1DFROM2D, '', '', 0, [  ])
  criteres = []
  unCritere = smesh.GetCriterion(SMESH.EDGE,SMESH.FT_FreeBorders,SMESH.FT_Undefined,0)
  criteres.append(unCritere)
  filtre = smesh.GetFilterFromCriteria(criteres)
  bordsLibres = internalBoundary.MakeGroupByFilter( 'bords', filtre )
  smesh.SetName(bordsLibres, 'bordsLibres')

  # --- pour aider l'algo hexa-tetra à ne pas mettre de pyramides à l'exterieur des volumes repliés sur eux-mêmes
  #     on désigne les faces de peau en quadrangles par le groupe "skinFaces"

  skinFaces = internalBoundary.CreateEmptyGroup( SMESH.FACE, 'skinFaces' )
  nbAdd = skinFaces.AddFrom( internalBoundary.GetMesh() )

  # --- maillage des éventuelles arêtes vives entre faces reconstruites
  
  if len(aretesVivesCoupees) > 0:
    aretesVivesC = geompy.MakeCompound(aretesVivesCoupees)
    meshAretesVives = smesh.Mesh(aretesVivesC)
    algo1d = meshAretesVives.Segment()
    hypo1d = algo1d.LocalLength(dmoyen,[],1e-07)
    putName(algo1d.GetSubMesh(), "aretesVives")
    putName(algo1d, "algo1d_aretesVives")
    putName(hypo1d, "hypo1d_aretesVives")
    isDone = meshAretesVives.Compute()
    logging.info("aretesVives fini")
    grpAretesVives = meshAretesVives.CreateEmptyGroup( SMESH.EDGE, 'grpAretesVives' )
    nbAdd = grpAretesVives.AddFrom( meshAretesVives.GetMesh() )

  # -----------------------------------------------------------------------
  # --- maillage faces de fissure
  
  logging.debug("---------------------------- maillage faces de fissure externes au pipe :%s --------------", len(facesFissExt))

  meshFaceFiss = smesh.Mesh(faceFissureExterne)
  algo2d = meshFaceFiss.Triangle(algo=smeshBuilder.NETGEN_1D2D)
  hypo2d = algo2d.Parameters()
  hypo2d.SetMaxSize( areteFaceFissure )
  hypo2d.SetSecondOrder( 0 )
  hypo2d.SetOptimize( 1 )
  hypo2d.SetFineness( 2 )
  hypo2d.SetMinSize( rayonPipe/float(nbsegRad) )
  hypo2d.SetQuadAllowed( 0 )
  putName(algo2d.GetSubMesh(), "faceFiss")
  putName(algo2d, "algo2d_faceFiss")
  putName(hypo2d, "hypo2d_faceFiss")
  
  algo1d = meshFaceFiss.UseExisting1DElements(geom=edgesPipeFissureExterneC)
  hypo1d = algo1d.SourceEdges([ edgeFaceFissGroup ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau")
  putName(algo1d, "algo1d_edgeFissPeau")
  putName(hypo1d, "hypo1d_edgeFissPeau")
  
  isDone = meshFaceFiss.Compute()
  logging.info("meshFaceFiss fini")

  grpFaceFissureExterne = meshFaceFiss.GroupOnGeom(faceFissureExterne, "fisOutPi", SMESH.FACE)
  grpEdgesPeauFissureExterne = meshFaceFiss.GroupOnGeom(edgesPeauFissureExterneC,'edgesPeauFissureExterne',SMESH.EDGE)
  grpEdgesPipeFissureExterne = meshFaceFiss.GroupOnGeom(edgesPipeFissureExterneC,'edgesPipeFissureExterne',SMESH.EDGE)

  # --- maillage faces de peau
    
  boutFromIfil = [None for i in range(nbFacesFilling)]
  if idFillingFromBout[0] != idFillingFromBout[1]: # repérage des extremites du pipe quand elles débouchent sur des faces différentes
    boutFromIfil[idFillingFromBout[0]] = 0
    boutFromIfil[idFillingFromBout[1]] = 1
  
  logging.debug("---------------------------- maillage faces de peau --------------")
  meshesFacesPeau = []
  for ifil in range(nbFacesFilling):
    meshFacePeau = None
    if partitionsPeauFissFond[ifil] is None: # face de peau maillage sain intacte
      
      # --- edges de bord de la face de filling
      filling = facesDefaut[ifil]
      edgesFilling = geompy.ExtractShapes(filling, geompy.ShapeType["EDGE"], False)
      groupEdgesBordPeau = geompy.CreateGroup(filling, geompy.ShapeType["EDGE"])
      geompy.UnionList(groupEdgesBordPeau, edgesFilling)
      geompy.addToStudyInFather(filling, groupEdgesBordPeau , "EdgesBords")
      
      meshFacePeau = smesh.Mesh(facesDefaut[ifil])
      
      algo1d = meshFacePeau.UseExisting1DElements(geom=groupEdgesBordPeau)
      hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
      putName(algo1d.GetSubMesh(), "bordsLibres", ifil)
      putName(algo1d, "algo1d_bordsLibres", ifil)
      putName(hypo1d, "hypo1d_bordsLibres", ifil)
      
    else:
      
      facePeau           = facesPeaux[ifil] # pour chaque face : la face de peau finale a mailler (percée des faces débouchantes)
      edgesCircPeau      = edCircPeau[ifil] # pour chaque face de peau : [subshape edge circulaire aux débouchés du pipe]
      verticesCircPeau   = ptCircPeau[ifil] # pour chaque face de peau : [subshape point sur edge circulaire aux débouchés du pipe]
      groupEdgesBordPeau = gpedgeBord[ifil] # pour chaque face de peau : groupe subshape des edges aux bords liés à la partie saine
      bordsVifs          = gpedgeVifs[ifil] # pour chaque face de peau : groupe subshape des edges aux bords correspondant à des arêtes vives
      edgesFissurePeau   = edFissPeau[ifil] # pour chaque face de peau : [subshape edge en peau des faces de fissure externes]

      meshFacePeau = smesh.Mesh(facePeau)
      
      algo1d = meshFacePeau.UseExisting1DElements(geom=groupEdgesBordPeau)
      hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
      putName(algo1d.GetSubMesh(), "bordsLibres", ifil)
      putName(algo1d, "algo1d_bordsLibres", ifil)
      putName(hypo1d, "hypo1d_bordsLibres", ifil)
      
      algo1d = meshFacePeau.UseExisting1DElements(geom=geompy.MakeCompound(edgesFissurePeau))
      hypo1d = algo1d.SourceEdges([ grpEdgesPeauFissureExterne ],0,0)
      putName(algo1d.GetSubMesh(), "edgePeauFiss", ifil)
      putName(algo1d, "algo1d_edgePeauFiss", ifil)
      putName(hypo1d, "hypo1d_edgePeauFiss", ifil)
      
      if bordsVifs is not None:
        algo1d = meshFacePeau.UseExisting1DElements(geom=bordsVifs)
        hypo1d = algo1d.SourceEdges([ grpAretesVives ],0,0)
        putName(algo1d.GetSubMesh(), "bordsVifs", ifil)
        putName(algo1d, "algo1d_bordsVifs", ifil)
        putName(hypo1d, "hypo1d_bordsVifs", ifil)
        
      for i, edgeCirc in enumerate(edgesCircPeau):
        if edgeCirc is not None:
          algo1d = meshFacePeau.UseExisting1DElements(geom=edgeCirc)
          if boutFromIfil[ifil] is None:
            hypo1d = algo1d.SourceEdges([ edgesCircPipeGroup[i] ],0,0)
          else:
            hypo1d = algo1d.SourceEdges([ edgesCircPipeGroup[boutFromIfil[ifil]] ],0,0)
          name = "cercle%d"%i
          putName(algo1d.GetSubMesh(), name, ifil)
          putName(algo1d, "algo1d_" + name, ifil)
          putName(hypo1d, "hypo1d_" + name, ifil)
   
    algo2d = meshFacePeau.Triangle(algo=smeshBuilder.NETGEN_1D2D)
    hypo2d = algo2d.Parameters()
    hypo2d.SetMaxSize( dmoyen )
    hypo2d.SetOptimize( 1 )
    hypo2d.SetFineness( 2 )
    hypo2d.SetMinSize( rayonPipe/float(nbsegRad) )
    hypo2d.SetQuadAllowed( 0 )
    putName(algo2d.GetSubMesh(), "facePeau", ifil)
    putName(algo2d, "algo2d_facePeau", ifil)
    putName(hypo2d, "hypo2d_facePeau", ifil)
      
    isDone = meshFacePeau.Compute()
    logging.info("meshFacePeau %d fini", ifil)
    GroupFaces = meshFacePeau.CreateEmptyGroup( SMESH.FACE, "facePeau%d"%ifil )
    nbAdd = GroupFaces.AddFrom( meshFacePeau.GetMesh() )
    meshesFacesPeau.append(meshFacePeau)

  # --- regroupement des maillages du défaut

  listMeshes = [internalBoundary.GetMesh(),
                meshPipe.GetMesh(),
                meshFaceFiss.GetMesh()]
  for mp in meshesFacesPeau:
    listMeshes.append(mp.GetMesh())

  meshBoiteDefaut = smesh.Concatenate(listMeshes, 1, 1, 1e-05,False)
  # pour aider l'algo hexa-tetra à ne pas mettre de pyramides à l'exterieur des volumes repliés sur eux-mêmes
  # on désigne les faces de peau en quadrangles par le groupe "skinFaces"
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
  putName(meshBoiteDefaut, "boiteDefaut")
  logging.info("meshBoiteDefaut fini")
  
  faceFissure = meshBoiteDefaut.GetMesh().UnionListOfGroups( [ group_faceFissOutPipe, group_faceFissInPipe ], 'FACE1' )
  maillageSain = enleveDefaut(maillageSain, zoneDefaut, zoneDefaut_skin,
                              zoneDefaut_internalFaces, zoneDefaut_internalEdges)
  putName(maillageSain, nomFicSain+"_coupe")
  extrusionFaceFissure, normfiss = shapeSurFissure(facesPortFissure)
  maillageComplet = RegroupeSainEtDefaut(maillageSain, meshBoiteDefaut,
                                         None, None, 'COMPLET', normfiss)

  logging.info("conversion quadratique")
  maillageComplet.ConvertToQuadratic( 1 )
  logging.info("groupes")
  groups = maillageComplet.GetGroups()
  grps = [ grp for grp in groups if grp.GetName() == 'FONDFISS']
  fond = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FONDFISS' )

  logging.info("réorientation face de fissure FACE1")
  grps = [ grp for grp in groups if grp.GetName() == 'FACE1']
  nb = maillageComplet.Reorient2D( grps[0], normfiss, grps[0].GetID(1))

  logging.info("réorientation face de fissure FACE2")
  plansim = geompy.MakePlane(O, normfiss, 10000)
  fissnorm = geompy.MakeMirrorByPlane(normfiss, plansim)
  grps = [ grp for grp in groups if grp.GetName() == 'FACE2']
  nb = maillageComplet.Reorient2D( grps[0], fissnorm, grps[0].GetID(1))
  fond = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FACE2' )

  logging.info("export maillage fini")
  maillageComplet.ExportMED( fichierMaillageFissure, 0, SMESH.MED_V2_2, 1 )
  putName(maillageComplet, nomFicFissure)
  logging.info("fichier maillage fissure %s", fichierMaillageFissure)

  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(1)

  logging.info("maillage fissure fini")
  
  return maillageComplet
