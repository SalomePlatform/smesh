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
"""Préparation maillage du pipe"""

import logging
import math

from .geomsmesh import geompy
from .geomsmesh import smesh

from .putName import putName

def calculePointsAxiauxPipe(edgesFondFiss, edgesIdByOrientation, facesDefaut,
                            centreFondFiss, wireFondFiss, wirePipeFiss,
                            lenSegPipe, rayonPipe, nbsegCercle, nbsegRad, \
                            nro_cas=None):
  """Préparation maillage du pipe :

  - détections des points a respecter : jonction des edges/faces constituant
    la face de fissure externe au pipe
  - points sur les edges de fond de fissure et edges pipe/face fissure,
  - vecteurs tangents au fond de fissure (normal au disque maillé)
  """

  logging.info('start')
  logging.info("Pour le cas n°%s", nro_cas)

  # --- option de maillage selon le rayon de courbure du fond de fissure
  lenEdgeFondExt = 0
  for edff in edgesFondFiss:
    lenEdgeFondExt += geompy.BasicProperties(edff)[0]

  disfond = list()
  for filling in facesDefaut:
    disfond.append(geompy.MinDistance(centreFondFiss, filling))
  disfond.sort()
  rcourb = disfond[0]
  texte = "rcourb: {}, lenEdgeFondExt: {}, lenSegPipe: {}".format(rcourb, lenEdgeFondExt, lenSegPipe)
  logging.info(texte)
  nbSegQuart = 5 # on veut 5 segments min sur un quart de cercle
  alpha = math.pi/(4*nbSegQuart)
  deflexion = rcourb*(1.0 -math.cos(alpha))
  lgmin = lenSegPipe*0.25
  lgmax = lenSegPipe*1.5
  texte = "==> deflexion: {}, lgmin: {}, lgmax: {}".format(deflexion, lgmin, lgmax)
  logging.info(texte)

  meshFondExt = smesh.Mesh(wireFondFiss)
  putName(meshFondExt, "wireFondFiss", i_pref=nro_cas)
  algo1d = meshFondExt.Segment()
  hypo1d = algo1d.Adaptive(lgmin, lgmax, deflexion) # a ajuster selon la profondeur de la fissure
  putName(algo1d.GetSubMesh(), "wireFondFiss", i_pref=nro_cas)
  putName(algo1d, "algo1d_wireFondFiss", i_pref=nro_cas)
  putName(hypo1d, "hypo1d_wireFondFiss", i_pref=nro_cas)

  is_done = meshFondExt.Compute()
  text = "calculePointsAxiauxPipe meshFondExt.Compute"
  if is_done:
    logging.info(text)
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  ptGSdic = dict() # dictionnaire [paramètre sur la courbe] --> point géométrique
  allNodeIds = meshFondExt.GetNodesId()
  for nodeId in allNodeIds:
    xyz = meshFondExt.GetNodeXYZ(nodeId)
    #logging.debug("nodeId %s, coords %s", nodeId, str(xyz))
    point = geompy.MakeVertex(xyz[0], xyz[1], xyz[2])
    parametre, _, EdgeInWireIndex = geompy.MakeProjectionOnWire(point, wireFondFiss) # parametre compris entre 0 et 1
    edgeOrder = edgesIdByOrientation[EdgeInWireIndex]
    ptGSdic[(edgeOrder, EdgeInWireIndex, parametre)] = point
    #logging.debug("nodeId %s, parametre %s", nodeId, str(parametre))
  usort = sorted(ptGSdic)
  logging.debug("nombre de points obtenus par deflexion %s",len(usort))

  centres = list()
  origins = list()
  normals = list()
  for edu in usort:
    vertcx = ptGSdic[edu]
    norm = geompy.MakeTangentOnCurve(edgesFondFiss[edu[1]], edu[2])
    plan = geompy.MakePlane(vertcx, norm, 3.*rayonPipe)
    part = geompy.MakePartition([plan], [wirePipeFiss], list(), list(), geompy.ShapeType["VERTEX"], 0, list(), 0)
    liste = geompy.ExtractShapes(part, geompy.ShapeType["VERTEX"], True)
    if ( len(liste) == 5 ): # 4 coins du plan plus intersection recherchée
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

  gptsdisks = list() # vertices géométrie de tous les disques
  raydisks = [list() for _ in range(nbsegCercle)]
  for indice, centres_i in enumerate(centres): # boucle sur les disques
    gptdsk = list() # vertices géométrie d'un disque
    vertcx = centres_i
    vertpx = origins[indice]
    normal = normals[indice]
    vec1 = geompy.MakeVector(vertcx, vertpx)

    points = [vertcx] # les points du rayon de référence
    dist_0 = rayonPipe/float(nbsegRad)
    for j_aux in range(nbsegRad):
      point = geompy.MakeTranslationVectorDistance(vertcx, vec1, float(j_aux+1)*dist_0)
      points.append(point)
    gptdsk.append(points)
    point = geompy.MakeTranslationVectorDistance(vertcx, vec1, 1.5*rayonPipe)
    rayon = geompy.MakeLineTwoPnt(vertcx, point)
    raydisks[0].append(rayon)

    angle_0 = 2.*math.pi/float(nbsegCercle)
    for k_aux in range(nbsegCercle-1):
      angle = float(k_aux+1)*angle_0
      pts = [vertcx] # les points d'un rayon obtenu par rotation
      for j_aux in range(nbsegRad):
        point = geompy.MakeRotation(points[j_aux+1], normal, angle)
        pts.append(point)
      gptdsk.append(pts)
      ray = geompy.MakeRotation(rayon, normal, angle)
      raydisks[k_aux+1].append(ray)

    gptsdisks.append(gptdsk)

  return (centres, gptsdisks, raydisks)
