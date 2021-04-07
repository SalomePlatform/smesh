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
"""Remarque : cette focntion n'est jamais appelée ????"""

import logging

import GEOM

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather

from . import initLog

from .listOfExtraFunctions import createNewMeshesFromCorner
from .listOfExtraFunctions import createLinesFromMesh

# -----------------------------------------------------------------------------
# --- groupe de quadrangles de face transformé en face géométrique par filling

def quadranglesToShapeWithCorner(meshQuad, shapeDefaut, listOfCorners):
  """TODO: rédiger la docstring"""

  logging.info("start")

  #fillings = [list(), list()]
  tmpFillings = list()
  noeuds_bords = list()
  #bords_Partages = [list(), list()]
  tmpBords = list()
  fillconts = list()
  idFilToCont = list()

  facesNonCoupees = list()
  facesCoupees = list()
  aretesNonCoupees = list()
  aretesCoupees = list()

  setOfNodes = list()
  setOfLines = list()
  listOfEdges = list()
  # On crée une liste contenant le maillage de chaque face.
  listOfNewMeshes = createNewMeshesFromCorner(meshQuad, listOfCorners)
  for msh in listOfNewMeshes:
    # On crée une liste de noeuds correspondant aux faces suivant
    # le modèle liste[face][ligne][noeud].
    #lines = createLinesFromMesh(msh, listOfCorners[0])
    lines = createLinesFromMesh(msh)
    setOfNodes.append(lines)

  for face in setOfNodes:
    tmpFace = list()
    for line in face:
      # On possède l'information 'ID' de chaque noeud composant chaque
      # ligne de chaque face. A partir de l'ID, on crée un vertex. Un
      # ensemble de vertices constitue une ligne. Un ensemble de lignes
      # constitue une face.
      tmpCoords = [meshQuad.GetNodeXYZ(node) for node in line]
      tmpPoints = [geompy.MakeVertex(val[0], val[1], val[2]) for val in tmpCoords]
      line = geompy.MakeInterpol(tmpPoints, False, False)
      tmpFace.append(line)
    setOfLines.append(tmpFace)

  for i_aux, face in enumerate(setOfLines):
    # A partir des lignes de chaque face,
    # on recrée un objet GEOM temporaire par filling.
    filling = geompy.MakeFilling(geompy.MakeCompound(face), 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default, True)
    geomPublish(initLog.debug, filling, 'filling_{}'.format(i_aux+1))
    tmpFillings.append(filling)

  for face in setOfNodes:
    # On prend la première ligne qui correspond aux bords partagés
    listOfEdges.append(face[0])

  for edge in listOfEdges:
    # On utilise les points de bords pour créer des aretes vives
    tmpCoords = [meshQuad.GetNodeXYZ(node) for node in list(edge)]
    tmpPoints = [geompy.MakeVertex(val[0], val[1], val[2]) for val in tmpCoords]
    line = geompy.MakeInterpol(tmpPoints, False, False)
    tmpBords.append(line)

  for i_aux, filling in enumerate(tmpFillings):
    tmpPartition = geompy.MakePartition([filling], [shapeDefaut], list(), list(), geompy.ShapeType["FACE"], 0, list(), 0, True)
    tmpExplodeRef = geompy.ExtractShapes(filling, geompy.ShapeType["EDGE"], True)
    tmpExplodeNum = geompy.ExtractShapes(tmpPartition, geompy.ShapeType["EDGE"], True)
    if len(tmpExplodeRef) == len(tmpExplodeNum):
      geomPublish(initLog.debug, filling, "faceNonCoupee_{}".format(i_aux+1))
      facesNonCoupees.append(filling)
    else:
      geomPublish(initLog.debug, filling, "faceCoupee_{}".format(i_aux+1))
      facesCoupees.append(filling)
  fillings = facesCoupees, facesNonCoupees

  for i_aux, filling in enumerate(tmpBords):
    tmpPartition = geompy.MakePartition([shapeDefaut], [filling], list(), list(), geompy.ShapeType["SHELL"], 0, list(), 0, True)
    tmpExplodeRef = geompy.ExtractShapes(shapeDefaut, geompy.ShapeType["EDGE"], True) + geompy.ExtractShapes(shapeDefaut, geompy.ShapeType["VERTEX"], True)
    tmpExplodeNum = geompy.ExtractShapes(tmpPartition, geompy.ShapeType["EDGE"], True) + geompy.ExtractShapes(tmpPartition, geompy.ShapeType["VERTEX"], True)
    if len(tmpExplodeRef) == len(tmpExplodeNum):
      geomPublish(initLog.debug, filling, "areteNonCoupee_{}".format(i_aux+1))
      aretesNonCoupees.append(filling)
    else:
      geomPublish(initLog.debug, filling, "areteCoupee_{}".format(i_aux+1))
      aretesCoupees.append(filling)
  bords_Partages = aretesCoupees, aretesNonCoupees

# TODO: A enlever
#    for i, face in enumerate(setOfLines):
#      for j, line in enumerate(face):
#      geomPublish(initLog.debug, line, 'face{0}_ligne{1}'.format(i + 1, j + 1))

#TODO: A enlever
#    for i, filling in enumerate(fillings[0]):
#      geomPublish(initLog.debug, filling, 'filling_{0}'.format(i + 1))
#      for j, line in enumerate(setOfLines[i]):
#      geompy.addToStudyInFather(filling, line, 'line_{0}'.format(j + 1))

  return fillings, noeuds_bords, bords_Partages, fillconts, idFilToCont
