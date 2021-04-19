# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of th79e GNU Lesser General Public
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
"""Groupe de quadrangles de face transformé en face géométrique par filling

Created on Tue Jun 24 09:14:13 2014
@author: I48174 (Olivier HOAREAU)
"""

import logging

import GEOM
import SMESH

from .geomsmesh import geompy
from .geomsmesh import smesh

from .listOfExtraFunctions import createNewMeshesFromCorner
from .listOfExtraFunctions import createLinesFromMesh
from .putName import putName

def fusionMaillageDefaut(maillageSain, maillageDefautCible, maillageInterneCible, \
                         zoneDefaut_skin, shapeDefaut, listOfCorners, \
                         nro_cas=None):
  """Groupe de quadrangles de face transformé en face géométrique par filling"""

  logging.info("start")
  logging.info("Pour le cas n°%s", nro_cas)

  facesNonCoupees = list()
  facesCoupees = list()
  maillagesNonCoupes = list()
  maillagesCoupes = list()

  # On crée une liste contenant le maillage de chaque face.
  listOfNewMeshes = createNewMeshesFromCorner(maillageDefautCible, listOfCorners, nro_cas)

  i_aux = 0
  while i_aux < len(listOfNewMeshes):
    lines = createLinesFromMesh(listOfNewMeshes[i_aux])
    setOfLines = list()
    for line in lines:
      # On possède l'information 'ID' de chaque noeud composant chaque
      # ligne de la face. A partir de l'ID, on crée un vertex. Un
      # ensemble de vertices constitue une ligne. Un ensemble de lignes
      # constitue la face.
      tmpCoords = [maillageDefautCible.GetNodeXYZ(node) for node in line]
      tmpPoints = [geompy.MakeVertex(val[0], val[1], val[2]) for val in tmpCoords]
      line = geompy.MakeInterpol(tmpPoints, False, False)
      setOfLines.append(line)

    # A partir des lignes de la face,
    # on recrée un objet GEOM temporaire par filling.
    filling = geompy.MakeFilling(geompy.MakeCompound(setOfLines), 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default, True)
    #logging.debug("face de filling")
    #geomPublish(initLog.debug, filling, 'filling_{0}'.format(i_aux+1))

    tmpPartition = geompy.MakePartition([filling], [shapeDefaut], list(), list(), geompy.ShapeType["FACE"], 0, list(), 0)
    tmpExplodeRef = geompy.ExtractShapes(filling, geompy.ShapeType["EDGE"], True)
    tmpExplodeNum = geompy.ExtractShapes(tmpPartition, geompy.ShapeType["EDGE"], True)
    if len(tmpExplodeRef) == len(tmpExplodeNum):
      logging.debug("face de filling non coupee")
      geompy.addToStudy( filling, "faceNonCoupee_{0}".format(i_aux+1)) # doit etre publié pour critère OK plus bas
      facesNonCoupees.append(filling)
      maillagesNonCoupes.append(listOfNewMeshes[i_aux])
    else:
      logging.debug("face de filling coupee")
      geompy.addToStudy( filling, "faceCoupee_{0}".format(i_aux+1))
      facesCoupees.append(filling)
      maillagesCoupes.append(listOfNewMeshes[i_aux])
    i_aux += 1

  listOfInternMeshes = [maillageInterneCible] + [msh.GetMesh() for msh in maillagesNonCoupes]

  newMaillageInterne = smesh.Concatenate(listOfInternMeshes, 1, 1, 1e-05, False)
  putName(newMaillageInterne, 'newInternalBoundary', i_pref=nro_cas)

  facesEnTrop = list()
  criteres = [smesh.GetCriterion(SMESH.FACE, SMESH.FT_BelongToGenSurface, SMESH.FT_Undefined, face) for face in facesNonCoupees]
  filtres = [smesh.GetFilterFromCriteria([critere]) for critere in criteres]
  for i_aux, filtre in enumerate(filtres):
    filtre.SetMesh(maillageSain.GetMesh())
    faceEnTrop = maillageSain.GroupOnFilter(SMESH.FACE, 'faceEnTrop_{0}'.format(i_aux+1), filtre)
    facesEnTrop.append(faceEnTrop)

  newZoneDefaut_skin = maillageSain.GetMesh().CutListOfGroups([zoneDefaut_skin], facesEnTrop, 'newZoneDefaut_skin')

  return newZoneDefaut_skin, newMaillageInterne
