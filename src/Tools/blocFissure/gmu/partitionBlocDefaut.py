# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

# -----------------------------------------------------------------------------
# --- partition du bloc defaut par generatrice, tore et plan fissure

def partitionBlocDefaut(volDefaut, facesDefaut, gener, pipe,
                        facefis, ellipsoide):
  """
  Partition du bloc defaut par la generatrice de l'ellipse,
  le tore elliptique, la face plane de fissure
  @param volDefaut : le bloc defaut
  @param gener : la generatrice de l'ellipse, positionnée dans l'espace
  @param pipe : le tore partitionné par le plan de fissure, positionné
  dans l'espace
  @param facefis : la face plane de la fissure, positionnée dans l'espace
  @return (volDefautPart, blocp, tore, faceFissure), le bloc partitionné,
  les sous shapes issues de la partition
  (le bloc moins le tore, la generatrice, le tore, la face de fissure)
  """
  logging.info("start")

  volDefautPart = geompy.MakePartition([volDefaut], [pipe, facefis, ellipsoide], [], [], geompy.ShapeType["SOLID"], 0, [], 1)
  blocp = geompy.GetInPlaceByHistory(volDefautPart, volDefaut)
  #gencnt = geompy.GetInPlaceByHistory(volDefautPart, gener)
  tore = geompy.GetInPlaceByHistory(volDefautPart, pipe)
  faceFissure = geompy.GetInPlaceByHistory(volDefautPart, facefis)
  #ellipsoidep =geompy.GetInPlaceByHistory(volDefautPart, ellipsoide)

  geomPublish(initLog.debug,  volDefautPart, 'volDefautPart' )
  geomPublishInFather(initLog.debug, volDefautPart, tore, 'tore' )
  geomPublishInFather(initLog.debug, volDefautPart, faceFissure, 'FACE1' )
  #geomPublishInFather(initLog.debug, volDefautPart, gencnt, 'generatrice' )

  solids = geompy.ExtractShapes(blocp, geompy.ShapeType["SOLID"], True)
  vols = []
  for i in range(len(solids)):
    props = geompy.BasicProperties(solids[i])
    vols.append(props[2])
  maxvol = max(vols)
  imaxvol = vols.index(maxvol)
  blocp = solids[imaxvol]
  vols[imaxvol] = 0
  maxvol = max(vols)
  imaxvol = vols.index(maxvol)
  ellipsoidep = solids[imaxvol]

  geomPublishInFather(initLog.debug, volDefautPart, blocp, 'bloc' )
  geomPublishInFather(initLog.debug, volDefautPart, ellipsoidep, 'ellipsoide' )

  sharedFaces = geompy.GetSharedShapesMulti([blocp, ellipsoidep], geompy.ShapeType["FACE"])
  for i in range(len(sharedFaces)):
    name = "faceCommuneEllipsoideBloc_%d"%i
    geomPublishInFather(initLog.debug,blocp, sharedFaces[i], name)

  #sharedEdges = geompy.GetSharedShapesMulti([blocp, ellipsoidep], geompy.ShapeType["EDGE"])
  allSharedEdges = geompy.GetSharedShapesMulti([blocp, ellipsoidep], geompy.ShapeType["EDGE"])
  sharedEdges = []
  for i in range(len(allSharedEdges)):
    if geompy.NbShapes(allSharedEdges[i], geompy.ShapeType["VERTEX"]) > 1: # edge non degeneree
      sharedEdges.append(allSharedEdges[i])
  for i in range(len(sharedEdges)):
    name = "edgeCommuneEllipsoideBloc_%d"%i
    geomPublishInFather(initLog.debug,blocp, sharedEdges[i], name)

  facesExternes = []
  facesExtBloc = []
  facesExtElli = []
  faces = geompy.ExtractShapes(facesDefaut, geompy.ShapeType["FACE"], True)
  if len(faces) == 0:
    faces = [facesDefaut]
  for i in range(len(faces)):
    faceExt = geompy.GetInPlace(ellipsoidep, faces[i])
    if faceExt is not None:
      name = "faceExterne_e%d"%i
      geomPublishInFather(initLog.debug,ellipsoidep, faceExt, name)
      facesExternes.append(faceExt)
      facesExtElli.append(faceExt)

    faceExt = geompy.GetInPlace(blocp, faces[i])
    if faceExt is not None:
      name = "faceExterne_b%d"%i
      geomPublishInFather(initLog.debug,blocp, faceExt, name)
      facesExternes.append(faceExt)
      facesExtBloc.append(faceExt)
    else:
      logging.info("  recherche faces externes par GetShapesOnShape")
      vertex = geompy.MakeVertexOnSurface(faces[i], 0.5, 0.5)
      normal = geompy.GetNormal(faces[i], vertex)
      extrusionFace = geompy.MakePrismVecH(faces[i], normal, 1)
      #extrusionFace = geompy.MakePrismVecH2Ways(faces[i], normal, 0.1)
      #extrusionFace = geompy.MakeScaleTransform(extrusionFace, vertex, 1.01)
      name = "extrusionFace_b%d"%i
      geomPublishInFather(initLog.debug,blocp, extrusionFace, name)
      #facesExt = geompy.GetShapesOnShape(extrusionFace, blocp, geompy.ShapeType["FACE"], GEOM.ST_ONIN)
      facesExt = geompy.GetShapesOnShape(extrusionFace, blocp, geompy.ShapeType["FACE"], GEOM.ST_ON)
      for j in range(len(facesExt)):
        name = "faceExterne_b%d_%d"%(i,j)
        geomPublishInFather(initLog.debug,blocp, facesExt[j], name)
        facesExternes.append(facesExt[j])
        facesExtBloc.append(facesExt[j])

  if len(facesExtBloc) < len(faces): # toutes les faces externes du bloc n'ont pas été trouvées. TODO eliminer les detections  multiples
    logging.info("  recherche faces externes par aretes partagees avec faces externes ellipsoide")
    facesBloc = geompy.ExtractShapes(blocp, geompy.ShapeType["FACE"], True)
    for i in range(len(facesBloc)):
      notOnEllipsoide = True
      for j in range(len(sharedFaces)): # eliminer les faces communes avec l'ellipsoide
        if facesBloc[i].IsSame(sharedFaces[j]):
          notOnEllipsoide = False
          break
      if notOnEllipsoide:
        for j in range(len(facesExtElli)): # les faces recherchees ont une ou plusieurs edge communes avec la ou les faces externes de l'ellipsoide
          allSharedEdges = []
          try:
            allSharedEdges += geompy.GetSharedShapesMulti([facesBloc[i], facesExtElli[j]], geompy.ShapeType["EDGE"])
          except:
            pass
          if len(allSharedEdges) > 0:
            name = "faceExterne_b%d_%d"%(i,j)
            geomPublishInFather(initLog.debug,blocp, facesBloc[i], name)
            facesExternes.append(facesBloc[i])
            facesExtBloc.append(facesBloc[i])

  aretesInternes = []
  for i in range(len(facesExternes)):
    for j in range(i+1,len(facesExternes)):
      shared = []
      try:
        shared += geompy.GetSharedShapesMulti([facesExternes[i], facesExternes[j]], geompy.ShapeType["EDGE"])
      except:
        logging.info("no shared edges in %s,%s",i,j)
      else:
        aretesInternes += shared
  for i in range(len(aretesInternes)):
    name = "aretesInternes_%d"%i
    geomPublishInFather(initLog.debug,blocp, aretesInternes[i], name)

  edgesBords = []
  for faceExtB in facesExtBloc:
    edges = geompy.ExtractShapes(faceExtB, geompy.ShapeType["EDGE"], True)
    for i in range(len(edges)):
      isInterne = False
      for j in range(len(aretesInternes)):
        if edges[i].IsSame(aretesInternes[j]):
          isInterne = True
          break
      if not isInterne:
        edgesBords.append(edges[i])
        name = "edgeBord%d"%i
        geomPublishInFather(initLog.debug,blocp,edges[i] , name)
  group = None
  if len(edgesBords) > 0:
    group = geompy.CreateGroup(blocp, geompy.ShapeType["EDGE"])
    geompy.UnionList(group, edgesBords)
  edgesBords = group

  return volDefautPart, blocp, tore, faceFissure, facesExternes, facesExtBloc, facesExtElli, aretesInternes, ellipsoidep, sharedFaces, sharedEdges, edgesBords
