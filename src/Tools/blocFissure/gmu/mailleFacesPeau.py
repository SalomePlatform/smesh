# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
from geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH

from putName import putName

def mailleFacesPeau(partitionsPeauFissFond, idFillingFromBout, facesDefaut,
                    facesPeaux, edCircPeau, ptCircPeau, gpedgeBord, gpedgeVifs, edFissPeau,
                    bordsLibres, grpEdgesPeauFissureExterne, grpAretesVives,
                    edgesCircPipeGroup, dmoyen, rayonPipe, nbsegRad):
  """
  maillage faces de peau
  """
  logging.info('start')
  nbFacesFilling = len(partitionsPeauFissFond)
  boutFromIfil = [None for i in range(nbFacesFilling)]
  if idFillingFromBout[0] != idFillingFromBout[1]: # repérage des extremites du pipe quand elles débouchent sur des faces différentes
    boutFromIfil[idFillingFromBout[0]] = 0
    boutFromIfil[idFillingFromBout[1]] = 1
  
  logging.debug("---------------------------- maillage faces de peau --------------")
  meshesFacesPeau = []
  for ifil in range(nbFacesFilling):
    meshFacePeau = None
    if partitionsPeauFissFond[ifil] is None: # face de peau maillage sain intacte

      logging.debug("meshFacePeau %d intacte", ifil)
      # --- edges de bord de la face de filling
      filling = facesDefaut[ifil]
      edgesFilling = geompy.ExtractShapes(filling, geompy.ShapeType["EDGE"], False)
      groupEdgesBordPeau = geompy.CreateGroup(filling, geompy.ShapeType["EDGE"])
      geompy.UnionList(groupEdgesBordPeau, edgesFilling)
      geomPublishInFather(initLog.debug,filling, groupEdgesBordPeau , "EdgesBords")
      
      meshFacePeau = smesh.Mesh(facesDefaut[ifil])
      
      algo1d = meshFacePeau.UseExisting1DElements(geom=groupEdgesBordPeau)
      hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
      putName(algo1d.GetSubMesh(), "bordsLibres", ifil)
      putName(algo1d, "algo1d_bordsLibres", ifil)
      putName(hypo1d, "hypo1d_bordsLibres", ifil)
      
    else:
      
      logging.debug("meshFacePeau %d coupée par la fissure", ifil)
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

  return meshesFacesPeau