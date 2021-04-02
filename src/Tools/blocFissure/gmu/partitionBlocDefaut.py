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
"""Partition du bloc defaut par generatrice, tore et plan fissure"""

import logging

import GEOM

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

from . import initLog

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
  vols = list()
  for solid in solids:
    props = geompy.BasicProperties(solid)
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
  for i_aux, face in enumerate(sharedFaces):
    name = "faceCommuneEllipsoideBloc_{}".format(i_aux)
    geomPublishInFather(initLog.debug,blocp, face, name)

  #sharedEdges = geompy.GetSharedShapesMulti([blocp, ellipsoidep], geompy.ShapeType["EDGE"])
  allSharedEdges = geompy.GetSharedShapesMulti([blocp, ellipsoidep], geompy.ShapeType["EDGE"])
  sharedEdges = list()
  for face in allSharedEdges:
    if geompy.NbShapes(face, geompy.ShapeType["VERTEX"]) > 1: # edge non degeneree
      sharedEdges.append(face)
  for i_aux, edge in enumerate(sharedEdges):
    name = "edgeCommuneEllipsoideBloc_{}".format(i_aux)
    geomPublishInFather(initLog.debug,blocp, edge, name)

  facesExternes = list()
  facesExtBloc = list()
  facesExtElli = list()
  faces = geompy.ExtractShapes(facesDefaut, geompy.ShapeType["FACE"], True)
  if not faces:
    faces = [facesDefaut]
  for i_aux, face in enumerate(faces):
    faceExt = geompy.GetInPlace(ellipsoidep, face)
    if faceExt is not None:
      name = "faceExterne_e{}".format(i_aux)
      geomPublishInFather(initLog.debug,ellipsoidep, faceExt, name)
      facesExternes.append(faceExt)
      facesExtElli.append(faceExt)

    faceExt = geompy.GetInPlace(blocp, face)
    if faceExt is not None:
      name = "faceExterne_b{}".format(i_aux)
      geomPublishInFather(initLog.debug,blocp, faceExt, name)
      facesExternes.append(faceExt)
      facesExtBloc.append(faceExt)
    else:
      logging.info("  recherche faces externes par GetShapesOnShape")
      vertex = geompy.MakeVertexOnSurface(face, 0.5, 0.5)
      normal = geompy.GetNormal(face, vertex)
      extrusionFace = geompy.MakePrismVecH(face, normal, 1)
      #extrusionFace = geompy.MakePrismVecH2Ways(face, normal, 0.1)
      #extrusionFace = geompy.MakeScaleTransform(extrusionFace, vertex, 1.01)
      name = "extrusionFace_b{}".format(i_aux)
      geomPublishInFather(initLog.debug,blocp, extrusionFace, name)
      #facesExt = geompy.GetShapesOnShape(extrusionFace, blocp, geompy.ShapeType["FACE"], GEOM.ST_ONIN)
      facesExt = geompy.GetShapesOnShape(extrusionFace, blocp, geompy.ShapeType["FACE"], GEOM.ST_ON)
      for j_aux, face_ext in enumerate(facesExt):
        name = "faceExterne_b{}_{}".format(i_aux,j_aux)
        geomPublishInFather(initLog.debug,blocp, face_ext, name)
        facesExternes.append(face_ext)
        facesExtBloc.append(face_ext)

  if len(facesExtBloc) < len(faces): # toutes les faces externes du bloc n'ont pas été trouvées. TODO eliminer les detections  multiples
    logging.info("  recherche faces externes par aretes partagees avec faces externes ellipsoide")
    facesBloc = geompy.ExtractShapes(blocp, geompy.ShapeType["FACE"], True)
    for i_aux, face in enumerate(facesBloc):
      notOnEllipsoide = True
      for j_aux, sharedface in enumerate(sharedFaces): # eliminer les faces communes avec l'ellipsoide
        if face.IsSame(sharedface):
          notOnEllipsoide = False
          break
      if notOnEllipsoide:
        for j_aux, face_ext_elli in enumerate(facesExtElli): # les faces recherchees ont une ou plusieurs edge communes avec la ou les faces externes de l'ellipsoide
          allSharedEdges = list()
          try:
            allSharedEdges += geompy.GetSharedShapesMulti([face, face_ext_elli], geompy.ShapeType["EDGE"])
          except:
            pass
          if allSharedEdges:
            name = "faceExterne_b{}_{}".format(i_aux,j_aux)
            geomPublishInFather(initLog.debug,blocp, face, name)
            facesExternes.append(face)
            facesExtBloc.append(face)

  aretesInternes = list()
  for i_aux, face_ext_i in enumerate(facesExternes):
    for j_aux, face_ext_j in enumerate(facesExternes[i_aux+1:]):
      shared = list()
      try:
        shared += geompy.GetSharedShapesMulti([face_ext_i, face_ext_j], geompy.ShapeType["EDGE"])
      except:
        texte = "no shared edges in {},{}".format(i_aux,j_aux)
        logging.info(texte)
      else:
        aretesInternes += shared
  for i_aux, edge in enumerate(aretesInternes):
    name = "aretesInternes_{}".format(i_aux)
    geomPublishInFather(initLog.debug,blocp, edge, name)

  l_edgesBords = list()
  for faceExtB in facesExtBloc:
    edges = geompy.ExtractShapes(faceExtB, geompy.ShapeType["EDGE"], True)
    for i_aux, edge in enumerate(edges):
      isInterne = False
      for arete in aretesInternes:
        if edge.IsSame(arete):
          isInterne = True
          break
      if not isInterne:
        l_edgesBords.append(edge)
        name = "edgeBord{}".format(i_aux)
        geomPublishInFather(initLog.debug,blocp,edge , name)
  if l_edgesBords:
    edgesBords = geompy.CreateGroup(blocp, geompy.ShapeType["EDGE"])
    geompy.UnionList(edgesBords, l_edgesBords)
  else:
    edgesBords = None

  return volDefautPart, blocp, tore, faceFissure, facesExternes, facesExtBloc, facesExtElli, aretesInternes, ellipsoidep, sharedFaces, sharedEdges, edgesBords
