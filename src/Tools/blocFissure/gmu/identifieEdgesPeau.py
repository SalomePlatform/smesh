# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

from substractSubShapes import substractSubShapes

def identifieEdgesPeau(edgesFissExtPipe,verticesPipePeau, facePeau, facesPeauSorted,
                       edgesPeauFondIn, fillingFaceExterne, aretesVivesC, aretesVivesCoupees):
  """
  identification précise des edges et disques des faces de peau selon index extremité fissure
  """
  logging.info('start')
  
  facesPipePeau = [None for i in range(len(edgesFissExtPipe))]
  endsEdgeFond = [None for i in range(len(edgesFissExtPipe))]
  edgeRadFacePipePeau = [None for i in range(len(edgesFissExtPipe))]
  
  edgesListees = []
  edgesCircPeau = []
  verticesCircPeau = []
  if len(verticesPipePeau) > 0: # --- au moins une extrémité du pipe sur cette face de peau
    
    for face in facesPeauSorted[:-1]: # la ou les faces débouchantes, pas la grande face de peau
      logging.debug("examen face debouchante circulaire")
      for i,efep in enumerate(edgesFissExtPipe):
        dist = geompy.MinDistance(face, efep)
        logging.debug("  distance face circulaire edge %s", dist)
        if dist < 1e-3:
          for ik, edpfi in enumerate(edgesPeauFondIn):
            if geompy.MinDistance(face, edpfi) < 1e-3:
              break
          sharedVertices = geompy.GetSharedShapesMulti([face, edgesPeauFondIn[ik]], geompy.ShapeType["VERTEX"])
          nameFace = "facePipePeau%d"%i
          nameVert = "endEdgeFond%d"%i
          nameEdge = "edgeRadFacePipePeau%d"%i
          facesPipePeau[i] = face
          endsEdgeFond[i] = sharedVertices[0]
          geomPublish(initLog.debug, face, nameFace)
          geomPublish(initLog.debug, sharedVertices[0], nameVert)
          edgesFace = geompy.ExtractShapes(face, geompy.ShapeType["EDGE"], True)
          for edge in edgesFace:
            if geompy.MinDistance(edge, sharedVertices[0]) < 1e-3:
              edgeRadFacePipePeau[i] = edge
              geomPublish(initLog.debug, edge, nameEdge)
              break
            pass
          pass
        pass
      pass
    
    # --- edges circulaires de la face de peau et points de jonction de la face externe de fissure
    logging.debug("facesPipePeau: %s", facesPipePeau)
    edgesCircPeau = [None for i in range(len(facesPipePeau))]
    verticesCircPeau = [None for i in range(len(facesPipePeau))]        
    for i,fcirc in enumerate(facesPipePeau):
      edges = geompy.GetSharedShapesMulti([facePeau, fcirc], geompy.ShapeType["EDGE"])
      grpEdgesCirc = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
      geompy.UnionList(grpEdgesCirc, edges)
      edgesCircPeau[i] = grpEdgesCirc
      name = "edgeCirc%d"%i
      geomPublishInFather(initLog.debug, facePeau, grpEdgesCirc, name)
      edgesListees = edgesListees + edges
      vertices = geompy.GetSharedShapesMulti([facePeau, fcirc], geompy.ShapeType["VERTEX"])
      grpVertCircPeau = geompy.CreateGroup(facePeau, geompy.ShapeType["VERTEX"])
      geompy.UnionList(grpVertCircPeau, vertices)
      verticesCircPeau[i] = grpVertCircPeau
      name = "pointEdgeCirc%d"%i
      geomPublishInFather(initLog.debug, facePeau, grpVertCircPeau, name)
      pass
    pass # --- au moins une extrémité du pipe sur cette face de peau

  # --- edges de bord de la face de peau

  edgesFilling = geompy.ExtractShapes(fillingFaceExterne, geompy.ShapeType["EDGE"], False)
  edgesBords = []
  for i, edge in enumerate(edgesFilling):
    edgepeau = geompy.GetInPlace(facePeau, edge)
    name = "edgepeau%d"%i
    geomPublishInFather(initLog.debug, facePeau,edgepeau, name)
    logging.debug("edgepeau %s", geompy.ShapeInfo(edgepeau))
    if geompy.ShapeInfo(edgepeau)['EDGE'] > 1:
      logging.debug("  EDGES multiples")
      edgs = geompy.ExtractShapes(edgepeau, geompy.ShapeType["EDGE"], False)
      edgesBords += edgs
      edgesListees += edgs
    else:
      logging.debug("  EDGE")
      edgesBords.append(edgepeau)
      edgesListees.append(edgepeau)
  groupEdgesBordPeau = geompy.CreateGroup(facePeau, geompy.ShapeType["EDGE"])
  geompy.UnionList(groupEdgesBordPeau, edgesBords)
  bordsVifs = None
  if aretesVivesC is not None:
    logging.debug("identification des bords vifs par GetInPlace (old)")
    bordsVifs = geompy.GetInPlace(facePeau, aretesVivesC)
  if bordsVifs is not None:
    geomPublishInFather(initLog.debug, facePeau, bordsVifs, "bordsVifs")
    groupEdgesBordPeau = geompy.CutGroups(groupEdgesBordPeau, bordsVifs)
    grptmp = None
    if len(aretesVivesCoupees) > 0:
      grpC = geompy.MakeCompound(aretesVivesCoupees)
      grptmp = geompy.GetInPlace(facePeau, grpC)
    if grptmp is not None:
      grpnew = geompy.CutGroups(bordsVifs, grptmp) # ce qui est nouveau dans bordsVifs
    else:
      grpnew = bordsVifs
    if grpnew is not None:
      edv = geompy.ExtractShapes(grpnew, geompy.ShapeType["EDGE"], False)
      aretesVivesCoupees += edv
  logging.debug("aretesVivesCoupees %s",aretesVivesCoupees)
  geomPublishInFather(initLog.debug, facePeau, groupEdgesBordPeau , "EdgesBords")
    
  # ---  edges de la face de peau partagées avec la face de fissure
  
  edgesPeau = geompy.ExtractShapes(facePeau, geompy.ShapeType["EDGE"], False)
  edges = substractSubShapes(facePeau, edgesPeau, edgesListees)
  edgesFissurePeau = []
  if len(verticesPipePeau) > 0: # --- au moins une extrémité du pipe sur cette face de peau
    edgesFissurePeau = [None for i in range(len(verticesCircPeau))] # edges associés aux extrémités du pipe, en premier
    for edge in edges:
      for i, grpVert in enumerate(verticesCircPeau):
        if (geompy.MinDistance(grpVert, edge) < 1.e-3) and (edge not in edgesFissurePeau):
          edgesFissurePeau[i] = edge
          name = "edgeFissurePeau%d"%i
          geomPublishInFather(initLog.debug, facePeau,  edge, name)
    for edge in edges: # on ajoute après les edges manquantes
      if edge not in edgesFissurePeau:
        edgesFissurePeau.append(edge)
  else:
    for i, edge in enumerate(edges):
      edgesFissurePeau.append(edge)
      name = "edgeFissurePeau%d"%i
      geomPublishInFather(initLog.debug, facePeau,  edge, name)
      
  return (endsEdgeFond, facesPipePeau, edgeRadFacePipePeau,
          edgesCircPeau, verticesCircPeau, groupEdgesBordPeau,
          bordsVifs, edgesFissurePeau, aretesVivesCoupees)
