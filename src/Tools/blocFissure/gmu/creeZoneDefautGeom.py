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
"""Zone de défaut, constructions géométrique avec CAO d'origine"""

import logging
from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .prolongeVertices import prolongeVertices

def creeZoneDefautGeom(objetSain, shapeDefaut, origShapes, verticesShapes, lgExtrusion=50):
  """Construction CAO de la zone à remailler, quand on utilise la CAO d'origine,apres appel creeZoneDefautMaillage

  @param objetSain : la géometrie de l'objet initial
  @param shapeDefaut : objet géometrique représentant la fissure
  (selon les cas, un point central, ou une shape plus complexe,
  dont on ne garde que les vertices)
  @param origShapes : liste id subShapes
  @param verticesShapes : listes noeuds de bord
  @lgExtrusion : distance d'extrusion de la face du defaut
  (ne vaut que pour des fissures courtes)
  @return (facesDefaut, centreDefaut, normalDefaut, extrusionDefaut)
  """
  logging.info("start")

  trace = True
  faces = list()
  curves = list()
  cdgs = list()
  projs = list()
  normals = list()
  extrusions = list()
  partitions = list()
  decoupes = list()

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
  #

  centreSphere = geompy.MakeCDG(shapeDefaut)
  geomPublish(initLog.debug, centreSphere, "cdg_defaut")
  ccurves = geompy.MakeCompound(curves)
  gravCenter = geompy.MakeCDG(ccurves)
  geomPublish(initLog.debug, gravCenter, "cdg_curves")
  for indice, part in enumerate(partitions):
    if trace:
      logging.debug(" --- original shape %s", origShapes[indice])
    dists = list()
    facesToSort = list()
    subFaces = geompy.ExtractShapes(part, geompy.ShapeType["FACE"], True)
    for aFace in subFaces:
      cdg = geompy.MakeCDG(aFace)
      distance = geompy.MinDistance(cdg, centreSphere)
      dists.append(distance)
      facesToSort.append(aFace)
      if trace:
        logging.debug("distance = %s", distance)
    if len(dists) > 0:
      minDist = min(dists)
      for j,d in enumerate(dists):
        if d == minDist:
          aFace = facesToSort[j]
          name="decoupe_%d"%origShapes[indice]
          geomPublish(initLog.debug, aFace, name)
          decoupes.append(aFace)
          break

  facesDefaut = decoupes[0]
  if len(decoupes) > 1:
    facesDefaut = geompy.MakePartition(decoupes, [], [], [], geompy.ShapeType["FACE"], 0, [], 0)
  geomPublish(initLog.debug, facesDefaut, "facesDefaut")

  shells = list()
  if len(decoupes) > 1: # plusieurs faces de defaut
    subFaces = geompy.ExtractShapes(facesDefaut, geompy.ShapeType["FACE"], True)
    # --- regroupe les subFaces en shells connectes
    theFaces = list(subFaces) # copy
    while theFaces:
      logging.debug("------- len(theFaces) %s" , len(theFaces))
      theFace = theFaces[0]
      logging.debug("  start with face %s",theFaces[0])
      theFaces[0:1] = list()
      aShell = [theFace]
      toAdd =[theFace]
      while toAdd:
        toAdd = list()
        toRemove = list()
        for indice, la_face in enumerate(theFaces):
          logging.debug("  try %s", la_face)
          for aFace in aShell:
            logging.debug("    with %s", aFace)
            try:
              _ = geompy.GetSharedShapesMulti([aFace, la_face], geompy.ShapeType["EDGE"])
              edgeShared = True
            except:
              edgeShared = False
            if edgeShared:
              if la_face not in toAdd:
                toAdd.append(la_face)
                toRemove.append(indice)
                logging.debug("    --- add %s", la_face)
        aShell += toAdd
        for k in sorted(toRemove, reverse=True):
          theFaces[k:k+1] = list()
      theShell = geompy.MakeShell(aShell)
      name = "theShell%d"%len(shells)
      geomPublish(initLog.debug, theShell,name)
      shells.append(theShell)
    #
    distances = list()
    for aShell in shells: # --- trouver le shell en contact avec la fissure
      distances.append(geompy.MinDistance(aShell, shapeDefaut))
    minDist = min(distances)
    for indice, dist in enumerate(distances):
      if dist == minDist:
        theShellDefaut = shells[indice]
        break
    #
  else: # --- une seule face de defaut
    subFaces = [facesDefaut]
    theShellDefaut = geompy.MakeShell(subFaces)
  if trace:
    geomPublish(initLog.debug, theShellDefaut,"theShellDefaut")

  theFaces = geompy.ExtractShapes(theShellDefaut, geompy.ShapeType["FACE"], True)
  distances = list()
  for aFace in theFaces:
    distances.append(geompy.MinDistance(aFace, centreSphere))
  minDist = min(distances)
  for indice, dist in enumerate(distances):
    if dist == minDist:
      indice0 = indice
      break

  centreDefaut = geompy.MakeProjection(centreSphere, theFaces[indice0])
  if trace:
    geomPublish(initLog.debug, centreDefaut, "centreDefaut")
  normalDefaut = geompy.GetNormal(subFaces[indice0], centreDefaut)
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
