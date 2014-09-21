# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import smesh
import SMESH

def construitMaillagePipe(gptsdisks, idisklim, nbsegCercle, nbsegRad):
  """
  maillage effectif du pipe
  """
  logging.info('start')
  meshPipe = smesh.Mesh(None, "meshPipe")
  fondFissGroup = meshPipe.CreateEmptyGroup(SMESH.EDGE, "FONDFISS")
  nodesFondFissGroup = meshPipe.CreateEmptyGroup(SMESH.NODE, "nfondfis")
  faceFissGroup = meshPipe.CreateEmptyGroup(SMESH.FACE, "fisInPi")
  edgeFaceFissGroup = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeFaceFiss")
  edgeCircPipe0Group = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeCircPipe0")
  edgeCircPipe1Group = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeCircPipe1")
  faceCircPipe0Group = meshPipe.CreateEmptyGroup(SMESH.FACE, "faceCircPipe0")
  faceCircPipe1Group = meshPipe.CreateEmptyGroup(SMESH.FACE, "faceCircPipe1")
  
  mptsdisks  = [] # vertices maillage de tous les disques
  mEdges     = [] # identifiants edges maillage fond de fissure
  mEdgeFaces = [] # identifiants edges maillage edge face de fissure externe
  mFaces     = [] # identifiants faces maillage fissure
  mVols      = [] # identifiants volumes maillage pipe

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

  meshPipeGroups = dict(fondFissGroup = fondFissGroup,
                        nodesFondFissGroup = nodesFondFissGroup,
                        faceFissGroup = faceFissGroup,
                        edgeFaceFissGroup = edgeFaceFissGroup,
                        edgeCircPipe0Group = edgeCircPipe0Group,
                        edgeCircPipe1Group = edgeCircPipe1Group,
                        faceCircPipe0Group = faceCircPipe0Group,
                        faceCircPipe1Group = faceCircPipe1Group,
                        pipeFissGroup = pipeFissGroup,
                        edgesCircPipeGroup = edgesCircPipeGroup
                        )
  
  return (meshPipe, meshPipeGroups, edgesCircPipeGroup)