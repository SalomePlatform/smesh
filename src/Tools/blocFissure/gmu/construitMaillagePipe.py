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
"""Maillage du pipe"""

import logging

import SMESH

from .geomsmesh import smesh
from .putName import putName

from .construitMaillagePipe_a import construitMaillagePipe_a
from .construitMaillagePipe_b import construitMaillagePipe_b
from .construitMaillagePipe_c import construitMaillagePipe_c
from .construitMaillagePipe_d import construitMaillagePipe_d

def construitMaillagePipe(gptsdisks, idisklim, nbsegCercle, nbsegRad, \
                          nro_cas=None):
  """maillage effectif du pipe"""
  logging.info('start')
  logging.info("nbsegCercle = %d, nbsegRad = %d", nbsegCercle, nbsegRad)
  logging.info("idisklim[0] = %d, idisklim[1] = %d", idisklim[0], idisklim[1])

  meshPipe = smesh.Mesh(None, "meshPipe")
  putName(meshPipe, "meshPipe", i_pref=nro_cas)

  edgesCircPipeGroup = list()

  fondFissGroup = meshPipe.CreateEmptyGroup(SMESH.EDGE, "FONDFISS")
  nodesFondFissGroup = meshPipe.CreateEmptyGroup(SMESH.NODE, "nfondfis")
  faceFissGroup = meshPipe.CreateEmptyGroup(SMESH.FACE, "fisInPi")
  edgeFaceFissGroup = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeFaceFiss")

  mptdsk     = list() # vertices de chaque disque au fur et à mesure
  mptsdisks  = list() # vertices maillage de tous les disques
  mEdges     = list() # identifiants edges maillage fond de fissure
  mEdgeFaces = list() # identifiants edges maillage edge face de fissure externe
  mFaces     = list() # identifiants faces maillage fissure
  mVols      = list() # identifiants volumes maillage pipe

  for idisk in range(idisklim[0], idisklim[1]+1): # boucle sur les disques internes
    #logging.info(". Prise en compte du disque n°%d", idisk)

    # -----------------------------------------------------------------------
    # --- Les points

    oldmpts = mptdsk
    mptdsk = construitMaillagePipe_a(idisk, \
                                      gptsdisks, nbsegCercle, \
                                      meshPipe, mptsdisks)

    # -----------------------------------------------------------------------
    # --- Les groupes des edges des cercles débouchants

    if idisk in (idisklim[0],idisklim[1]):
      construitMaillagePipe_b(idisk, \
                              idisklim, nbsegCercle, \
                              meshPipe, mptdsk, \
                              edgesCircPipeGroup)

    # -----------------------------------------------------------------------
    # --- Les groupes des faces débouchantes

    if idisk in (idisklim[0],idisklim[1]):
      construitMaillagePipe_c(idisk, \
                              idisklim, nbsegCercle, \
                              meshPipe, mptdsk, nbsegRad)

    # -----------------------------------------------------------------------
    # --- mailles volumiques, groupes noeuds et edges de fond de fissure, groupe de face de fissure

    construitMaillagePipe_d(idisk, \
                            idisklim, nbsegCercle, nbsegRad, \
                            meshPipe, mptdsk, oldmpts, \
                            fondFissGroup, edgeFaceFissGroup, faceFissGroup, \
                            mEdges, mEdgeFaces, mFaces, mVols, nodesFondFissGroup)
  # Bilan

  pipeFissGroup = meshPipe.CreateEmptyGroup( SMESH.VOLUME, 'PIPEFISS' )
  _ = pipeFissGroup.AddFrom( meshPipe.GetMesh() )

  _, _, _ = meshPipe.MakeBoundaryElements(SMESH.BND_2DFROM3D, "pipeBoundaries")

  #if meshPipe:
    #text = "Arrêt rapide.\n"
    #logging.info(text)
    #raise Exception(text)

  return (meshPipe, edgeFaceFissGroup, edgesCircPipeGroup)
