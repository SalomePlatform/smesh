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
"""Les groupes de la fissure longue"""

import logging

import SMESH

from .geomsmesh import geompy

from .putName import putName
from .enleveDefaut import enleveDefaut
from .shapeSurFissure import shapeSurFissure
from .regroupeSainEtDefaut import RegroupeSainEtDefaut
from .triedreBase import triedreBase

# -----------------------------------------------------------------------------

def insereFissureLongue_g(nomFicFissure, fichierMaillageFissure, nomFicSain, maillageSain, \
                          meshBoiteDefaut, facePorteFissure, \
                          group_faceFissInPipe, group_faceFissOutPipe, \
                          zoneDefaut, zoneDefaut_skin, zoneDefaut_internalEdges, zoneDefaut_internalFaces, \
                          nro_cas=None):
  """Les groupes de la fissure longue"""
  logging.info('start')
  logging.info("Pour le cas n°%s", nro_cas)

  O, _, _, _ = triedreBase()

  groups = maillageSain.GetGroups()
  grps1 = [ grp for grp in groups if grp.GetName() == 'P1']
  grps2 = [ grp for grp in groups if grp.GetName() == 'P2']
  coords1 = maillageSain.GetNodeXYZ(grps1[0].GetID(1))
  coords2 = maillageSain.GetNodeXYZ(grps2[0].GetID(1))
  logging.info("coords1 %s, coords2 %s",coords1, coords2)

  _ = meshBoiteDefaut.GetMesh().UnionListOfGroups( [ group_faceFissOutPipe, group_faceFissInPipe ], 'FACE1' )
  maillageSain = enleveDefaut(maillageSain, zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges)
  putName(maillageSain, nomFicSain+"_coupe", i_pref=nro_cas)
  extrusionFaceFissure, normfiss = shapeSurFissure(facePorteFissure)
  maillageComplet = RegroupeSainEtDefaut(maillageSain, meshBoiteDefaut, extrusionFaceFissure, facePorteFissure, 'COUDE')

  groups = maillageComplet.GetGroups()
  grps1 = [ grp for grp in groups if grp.GetName() == 'P1']
  grps2 = [ grp for grp in groups if grp.GetName() == 'P2']
  nodeid1 = maillageComplet.AddNode(coords1[0], coords1[1], coords1[2])
  nodeid2 = maillageComplet.AddNode(coords2[0], coords2[1], coords2[2])
  grps1[0].Add([nodeid1])
  grps2[0].Add([nodeid2])
  ma0d1 = maillageComplet.Add0DElement(nodeid1)
  ma0d2 = maillageComplet.Add0DElement(nodeid2)
  grpma0d1 = maillageComplet.CreateEmptyGroup( SMESH.ELEM0D, 'P1' )
  _ = grpma0d1.Add( [ma0d1] )
  grpma0d2 = maillageComplet.CreateEmptyGroup( SMESH.ELEM0D, 'P2' )
  _ = grpma0d2.Add( [ma0d2] )

#  grps = [ grp for grp in groups if grp.GetName() == 'affectedEdges']
#  grps[0].SetName('affEdges')
#  grps = [ grp for grp in groups if grp.GetName() == 'affectedFaces']
#  grps[0].SetName('affFaces')
#  grps = [ grp for grp in groups if grp.GetName() == 'affectedVolumes']
#  grps[0].SetName('affVols')

  maillageComplet.ConvertToQuadratic( 1 )
  grps = [ grp for grp in groups if grp.GetName() == 'FONDFISS']
  _ = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FONDFISS' )

  grps = [ grp for grp in groups if grp.GetName() == 'FACE1']
  _ = maillageComplet.Reorient2D( grps[0], normfiss, grps[0].GetID(1))

  plansim = geompy.MakePlane(O, normfiss, 10000)
  fissnorm = geompy.MakeMirrorByPlane(normfiss, plansim)
  grps = [ grp for grp in groups if grp.GetName() == 'FACE2']
  _ = maillageComplet.Reorient2D( grps[0], fissnorm, grps[0].GetID(1))
  #isDone = maillageComplet.ReorientObject( grps[0] )
  _ = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FACE2' )

  maillageComplet.ExportMED(fichierMaillageFissure)
  putName(maillageComplet, nomFicFissure)
  logging.info("fichier maillage fissure %s", fichierMaillageFissure)

  return maillageComplet
