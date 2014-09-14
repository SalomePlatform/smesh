# -*- coding: utf-8 -*-

import logging
import math

from geomsmesh import geompy
from geomsmesh import smesh
  
def calculePointsAxiauxPipe(edgesFondFiss, edgesIdByOrientation, facesDefaut, 
                            centreFondFiss, wireFondFiss, wirePipeFiss,
                            lenSegPipe, rayonPipe, nbsegCercle, nbsegRad):
  """
  preparation maillage du pipe :
  - détections des points a respecter : jonction des edges/faces constituant
    la face de fissure externe au pipe
  - points sur les edges de fond de fissure et edges pipe/face fissure,
  - vecteurs tangents au fond de fissure (normal au disque maillé)  
  """
  
  logging.info('start')

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
    
  return (centres, gptsdisks, raydisks)
