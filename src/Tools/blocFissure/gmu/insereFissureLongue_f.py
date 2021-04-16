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
"""Insertion de fissure longue - maillage meshBoiteDefaut"""

import logging

from salome.smesh import smeshBuilder
import SMESH

from .geomsmesh import smesh

from .putName import putName

def insereFissureLongue_f (internalBoundary, meshFondFiss, meshFacePeau, meshFaceFiss, \
                           mailleur="MeshGems", nro_cas=None):
  """maillage meshBoiteDefaut"""
  logging.info('start')
  logging.info("Usage du mailleur %s pour le cas n°%s", mailleur, nro_cas)

  meshBoiteDefaut = smesh.Concatenate( [internalBoundary.GetMesh(), \
                                        meshFondFiss.GetMesh(), \
                                        meshFacePeau.GetMesh(), \
                                        meshFaceFiss.GetMesh()], \
                                        1, 1, 1e-05,False )
  # pour aider l'algo hexa-tetra a ne pas mettre de pyramides a l'exterieur des volumes replies sur eux-memes
  # on designe les faces de peau en quadrangles par le groupe "skinFaces"
  group_faceFissOutPipe = None
  group_faceFissInPipe = None
  groups = meshBoiteDefaut.GetGroups()
  for grp in groups:
    if grp.GetType() == SMESH.FACE:
      #if "internalBoundary" in grp.GetName():
      #  grp.SetName("skinFaces")
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
  putName(meshBoiteDefaut, "boiteDefaut", i_pref=nro_cas)

  is_done = meshBoiteDefaut.Compute()
  text = "meshBoiteDefaut.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  return meshBoiteDefaut, group_faceFissInPipe, group_faceFissOutPipe
