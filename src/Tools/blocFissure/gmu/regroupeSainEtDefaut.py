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
"""Maillage sain sans la zone de defaut"""

import logging

import SMESH

from .geomsmesh import geompy
from .geomsmesh import smesh

def RegroupeSainEtDefaut(maillageSain, blocComplet, extrusionFaceFissure, faceGeomFissure, nomVolume, normal = None):
  """Maillage sain sans la zone de defaut"""
  logging.info('Concatenation')

  maillageComplet = smesh.Concatenate([maillageSain.GetMesh(), blocComplet.GetMesh()], 1, 1, 1e-05,False)

  groups = maillageComplet.GetGroups()
  for grp in groups:
    grp_nom = grp.GetName()
    if ( grp_nom == "FACE1" ):
      faceFissure = grp
    elif ( grp_nom == "nfondfis" ):
      noeudsFondFissure = grp
    elif ( grp_nom == "fisInPi" ):
      fisInPi = grp
    elif ( grp_nom == "fisOutPi" ):
      fisOutPi = grp

  # --- TODO: fiabiliser l'orientation dans le cas general
  if normal is None:
    normal  = smesh.MakeDirStruct( 0, 0, 1 )
  texte = 'après normal = {}'.format(normal)
  logging.debug(texte)
  maillageComplet.Reorient2D( fisInPi,  normal, [0,0,0])
  logging.debug('après Reorient2D In')
  maillageComplet.Reorient2D( fisOutPi, normal, [0,0,0])

  shapes = list()
  if extrusionFaceFissure is not None:
    subIds = geompy.SubShapeAllIDs(extrusionFaceFissure, geompy.ShapeType["SOLID"])
    if ( len(subIds) > 1 ):
      shapes = geompy.ExtractShapes(extrusionFaceFissure, geompy.ShapeType["SOLID"], False)
    else:
      shapes = [extrusionFaceFissure]
#  else:
#    subIds = geompy.SubShapeAllIDs(faceGeomFissure, geompy.ShapeType["FACE"])
#    if len(subIds) > 1:
#      shapes = geompy.ExtractShapes(faceGeomFissure, geompy.ShapeType["FACE"], False)
#    else:
#      shapes = [faceGeomFissure]

  grpEdges = list()
  grpFaces = list()
  grpVolumes = list()
  if not shapes:
    shapes = [None] # calcul uniquement avec les normales des faces mailles de la fissure
  for i, aShape in enumerate(shapes):
    texte = "Detection elements affectes par le dedoublement de la face n° {}".format(i)
    logging.debug(texte)
    affectedGroups = maillageComplet.AffectedElemGroupsInRegion([faceFissure], [noeudsFondFissure], aShape)
    for grp in affectedGroups:
      grp_nom = grp.GetName()
      if ( grp_nom == "affectedEdges" ):
        affectedEdges = grp
      elif ( grp_nom == "affectedFaces" ):
        affectedFaces = grp
      elif ( grp_nom == "affectedVolumes" ):
        affectedVolumes = grp
    #grps = [ grp for grp in affectedGroups if grp.GetName() == 'affectedEdges']
    #affectedEdges = grps[0]
    affectedEdges.SetName('affEd%d'%i)
    grpEdges.append(affectedEdges)
    #grps = [ grp for grp in affectedGroups if grp.GetName() == 'affectedFaces']
    #affectedFaces = grps[0]
    affectedFaces.SetName('affFa%d'%i)
    grpFaces.append(affectedFaces)
    #grps = [ grp for grp in affectedGroups if grp.GetName() == 'affectedVolumes']
    #affectedVolumes = grps[0]
    affectedVolumes.SetName('affVo%d'%i)
    grpVolumes.append(affectedVolumes)
  affectedEdges = maillageComplet.UnionListOfGroups(grpEdges, 'affEdges')
  affectedFaces = maillageComplet.UnionListOfGroups(grpFaces, 'affFaces')
  affectedVolumes = maillageComplet.UnionListOfGroups(grpVolumes, 'affVols')
  for grp in affectedGroups:
    texte = "Traitement du groupe '{}'".format(grp.GetName())
    logging.debug(texte)
  [ FACE2, _ ] = maillageComplet.DoubleNodeElemGroups([faceFissure], [noeudsFondFissure], affectedGroups, True, True)
  FACE2.SetName( 'FACE2' )

  # Groupe de toutes les mailles volumiques
  GroupVol = maillageComplet.CreateEmptyGroup( SMESH.VOLUME, nomVolume )
  _ = GroupVol.AddFrom( maillageComplet.GetMesh() )

  return maillageComplet
