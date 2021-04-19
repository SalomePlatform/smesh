# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
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
"""construction de la fissure générale - maillage"""

import os

import logging

from salome.smesh import smeshBuilder
import SMESH

from .geomsmesh import geompy

from .putName import putName
from .enleveDefaut import enleveDefaut
from .shapeSurFissure import shapeSurFissure
from .regroupeSainEtDefaut import RegroupeSainEtDefaut
from .triedreBase import triedreBase

# -----------------------------------------------------------------------------

def construitFissureGenerale_c(maillageSain, meshBoiteDefaut, \
                              zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges, \
                              facesPortFissure, \
                              maillageFissureParams, \
                              mailleur="MeshGems", nro_cas=None):
  """construction de la fissure générale - maillage"""
  logging.info('start')
  logging.info("Usage du mailleur %s pour le cas n°%s", mailleur, nro_cas)

  nomRep            = maillageFissureParams['nomRep']
  nomFicSain        = maillageFissureParams['nomFicSain']
  nomFicFissure     = maillageFissureParams['nomFicFissure']

  O, _, _, _ = triedreBase()

  # pour aider l'algo hexa-tetra à ne pas mettre de pyramides à l'exterieur des volumes repliés sur eux-mêmes
  # on désigne les faces de peau en quadrangles par le groupe "skinFaces"
  group_faceFissOutPipe = None
  group_faceFissInPipe = None
  groups = meshBoiteDefaut.GetGroups()
  for grp in groups:
    if grp.GetType() == SMESH.FACE:
      if grp.GetName() == "fisOutPi":
        group_faceFissOutPipe = grp
      elif grp.GetName() == "fisInPi":
        group_faceFissInPipe = grp

  # le maillage NETGEN ne passe pas toujours ==> on force l'usage de MG_Tetra
  mailleur = "MeshGems"
  logging.info("Maillage avec %s", mailleur)
  if ( mailleur == "MeshGems"):
    algo3d = meshBoiteDefaut.Tetrahedron(algo=smeshBuilder.MG_Tetra)
  else:
    algo3d = meshBoiteDefaut.Tetrahedron(algo=smeshBuilder.NETGEN)
    hypo3d = algo3d.MaxElementVolume(1000.0)
    hypo3d.SetVerboseLevel( 0 )
    hypo3d.SetStandardOutputLog( 0 )
    hypo3d.SetRemoveLogOnSuccess( 1 )
  putName(algo3d.GetSubMesh(), "boiteDefaut", i_pref=nro_cas)

  is_done = meshBoiteDefaut.Compute()
  text = "meshBoiteDefaut.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  _ = meshBoiteDefaut.GetMesh().UnionListOfGroups( [ group_faceFissOutPipe, group_faceFissInPipe ], \
                                                             'FACE1' )
  maillageSain = enleveDefaut(maillageSain, zoneDefaut, zoneDefaut_skin,
                              zoneDefaut_internalFaces, zoneDefaut_internalEdges)
  putName(maillageSain, nomFicSain+"_coupe", i_pref=nro_cas)
  _, normfiss = shapeSurFissure(facesPortFissure)
  maillageComplet = RegroupeSainEtDefaut(maillageSain, meshBoiteDefaut, \
                                         None, None, 'COMPLET', normfiss)
  putName(maillageComplet, nomFicFissure)

  logging.info("conversion quadratique")
  maillageComplet.ConvertToQuadratic( 1 )

  logging.info("groupes")
  groups = maillageComplet.GetGroups()
  grps = [ grp for grp in groups if grp.GetName() == 'FONDFISS']
  _ = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FONDFISS' )

  logging.info("réorientation face de fissure FACE1")
  grps = [ grp for grp in groups if grp.GetName() == 'FACE1']
  _ = maillageComplet.Reorient2D( grps[0], normfiss, grps[0].GetID(1))

  logging.info("réorientation face de fissure FACE2")
  plansim = geompy.MakePlane(O, normfiss, 10000)
  fissnorm = geompy.MakeMirrorByPlane(normfiss, plansim)
  grps = [ grp for grp in groups if grp.GetName() == 'FACE2']
  _ = maillageComplet.Reorient2D( grps[0], fissnorm, grps[0].GetID(1))
  _ = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FACE2' )

  logging.info("export maillage fini")
  fichierMaillageFissure = os.path.join (nomRep , '{}.med'.format(nomFicFissure))
  maillageComplet.ExportMED(fichierMaillageFissure)
  logging.info("fichier maillage fissure %s", fichierMaillageFissure)

  return maillageComplet
