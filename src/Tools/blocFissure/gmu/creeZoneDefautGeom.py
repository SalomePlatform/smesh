# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
from prolongeVertices import prolongeVertices

# -----------------------------------------------------------------------------
# --- zone de defaut, constructions geometrique avec CAO d'origine

def creeZoneDefautGeom(objetSain, shapeDefaut, origShapes, verticesShapes, dmoyen, lgExtrusion=50):
  """
  Construction CAO de la zone à remailler, quand on utilise la CAO d'origine,
  apres appel creeZoneDefautMaillage
  @param objetSain : la géometrie de l'objet initial
  @param shapeDefaut : objet géometrique représentant la fissure
  (selon les cas, un point central, ou une shape plus complexe,
  dont on ne garde que les vertices)
  @param origShapes : liste id subShapes
  @param verticesShapes : listes noeuds de bord
  @param dmoyen : longueur arete moyenne bord
  @lgExtrusion : distance d'extrusion de la face du defaut
  (ne vaut que pour des fissures courtes)
  @return (facesDefaut, centreDefaut, normalDefaut, extrusionDefaut)
  """
  logging.info("start")

  trace = True
  faces = []
  curves = []
  cdgs = []
  projs = []
  normals = []
  extrusions = []
  partitions = []
  decoupes = []

  for ishape, vertices in enumerate(verticesShapes):
    aShape = origShapes[ishape]
    [face] = geompy.SubShapes(objetSain, [aShape])
    faces.append(face)
    curve = geompy.MakePolyline(vertices, False)
    curves.append(curve)
    if trace:
      name="poly_%d"%aShape
      geomPublish(initLog.debug, curve, name)
    #
    cdg = geompy.MakeCDG(curve)
    cdgs.append(cdg)
    if trace:
      name="cdgpoly_%d"%aShape
      geomPublish(initLog.debug, cdg, name)
    #
    projCdg = geompy.MakeProjection(cdg, face)
    projs.append(projCdg)
    if trace:
      name="projCdg_%d"%aShape
      geomPublish(initLog.debug, projCdg, name)
    #
    normal = geompy.GetNormal(face, projCdg)
    normals.append(normal)
    if trace:
      name="normal_%d"%aShape
      geomPublish(initLog.debug, normal, name)
    #
    extrusion = geompy.MakePrismVecH2Ways(curve, normal, 10)
    extrusions.append(extrusion)
    if trace:
      name="extrusion_%d"%aShape
      geomPublish(initLog.debug, extrusion, name)
    #
    verticesProlongees = prolongeVertices(vertices)
    #
    curveprol = geompy.MakePolyline(verticesProlongees, False)
    if trace:
      name="polyProl_%d"%aShape
      geomPublish(initLog.debug, curveprol, name)
    #
    extruprol = geompy.MakePrismVecH2Ways(curveprol, normal, 10)
    if trace:
      name="extruProl_%d"%aShape
      geomPublish(initLog.debug, extruprol, name)
    #
    partition = geompy.MakePartition([face], [extruprol], [], [], geompy.ShapeType["FACE"], 0, [], 0)
    partitions.append(partition)
    if trace:
      name="partition_%d"%aShape
      geomPublish(initLog.debug, partition, name)
    pass
  #

  centreSphere = geompy.MakeCDG(shapeDefaut)
  geomPublish(initLog.debug, centreSphere, "cdg_defaut")
  ccurves = geompy.MakeCompound(curves)
  gravCenter = geompy.MakeCDG(ccurves)
  geomPublish(initLog.debug, gravCenter, "cdg_curves")
  for i in range(len(partitions)):
    if trace:
      logging.debug(" --- original shape %s", origShapes[i])
    dists = []
    facesToSort = []
    subFaces = geompy.ExtractShapes(partitions[i], geompy.ShapeType["FACE"], True)
    for aFace in subFaces:
      cdg = geompy.MakeCDG(aFace)
      distance = geompy.MinDistance(cdg, centreSphere)
      dists.append(distance)
      facesToSort.append(aFace)
      if trace:
        logging.debug("distance = %s", distance)
        pass
      pass
    if len(dists) > 0:
      minDist = min(dists)
      for j,d in enumerate(dists):
        if d == minDist:
          aFace = facesToSort[j]
          name="decoupe_%d"%origShapes[i]
          geomPublish(initLog.debug, aFace, name)
          decoupes.append(aFace)
          break
        pass
      pass

  facesDefaut = decoupes[0]
  if len(decoupes) > 1:
    facesDefaut = geompy.MakePartition(decoupes, [], [], [], geompy.ShapeType["FACE"], 0, [], 0)
  geomPublish(initLog.debug, facesDefaut, "facesDefaut")

  shells=[]
  if len(decoupes) > 1: # plusieurs faces de defaut
    subFaces = geompy.ExtractShapes(facesDefaut, geompy.ShapeType["FACE"], True)
    # --- regroupe les subFaces en shells connectes
    theFaces = list(subFaces) # copy
    while len(theFaces) > 0:
      logging.debug("------- len(theFaces) %s" , len(theFaces))
      theFace = theFaces[0]
      logging.debug("  start with face %s",theFaces[0])
      theFaces[0:1] = []
      aShell = [theFace]
      toAdd =[theFace]
      while len(toAdd) > 0:
        toAdd = []
        toRemove = []
        for i in range(len(theFaces)):
          logging.debug("  try %s", theFaces[i])
          for aFace in aShell:
            logging.debug("    with %s", aFace)
            try:
              edge = geompy.GetSharedShapesMulti([aFace, theFaces[i]], geompy.ShapeType["EDGE"])
              edgeShared = True
            except:
              edgeShared = False
            if edgeShared:
              if theFaces[i] not in toAdd:
                toAdd.append(theFaces[i])
                toRemove.append(i)
                logging.debug("    --- add %s", theFaces[i])
        aShell += toAdd
        for k in sorted(toRemove, reverse=True):
          theFaces[k:k+1] = []
      theShell = geompy.MakeShell(aShell)
      name = "theShell%d"%len(shells)
      geomPublish(initLog.debug, theShell,name)
      shells.append(theShell)
    #
    distances = []
    for aShell in shells: # --- trouver le shell en contact avec la fissure
      distances.append(geompy.MinDistance(aShell, shapeDefaut))
    minDist = min(distances)
    for index in range(len(distances)):
      if distances[index] == minDist:
        break
    theShellDefaut = shells[index]
    #
  else: # --- une seule face de defaut
    subFaces = [facesDefaut]
    theShellDefaut = geompy.MakeShell(subFaces)
  if trace:
    geomPublish(initLog.debug, theShellDefaut,"theShellDefaut")

  theFaces = geompy.ExtractShapes(theShellDefaut, geompy.ShapeType["FACE"], True)
  distances = []
  for aFace in theFaces:
    distances.append(geompy.MinDistance(aFace, centreSphere))
  minDist = min(distances)
  for index in range(len(distances)):
    if distances[index] == minDist:
      break

  centreDefaut = geompy.MakeProjection(centreSphere, theFaces[index])
  if trace:
    geomPublish(initLog.debug, centreDefaut, "centreDefaut")
  normalDefaut = geompy.GetNormal(subFaces[index], centreDefaut)
  if trace:
    geomPublish(initLog.debug, normalDefaut, "normalDefaut")
  extrusionDefaut = geompy.MakePrismVecH(theShellDefaut, normalDefaut, -lgExtrusion)
  info = geompy.ShapeInfo(extrusionDefaut)
  logging.debug("shape info %s", info)
  if (info['SOLID'] > 1) or (info['COMPOUND'] > 0) :
    solids = geompy.ExtractShapes(extrusionDefaut, geompy.ShapeType["SOLID"], True)
    solid0 = solids[0]
    for i in range(1,len(solids)):
      solid0 = geompy.MakeFuse(solid0, solids[i])
    extrusionDefaut = solid0
  if trace:
    geomPublish(initLog.debug, extrusionDefaut, "extrusionDefaut")

  return facesDefaut, centreDefaut, normalDefaut, extrusionDefaut
