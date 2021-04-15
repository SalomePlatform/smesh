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
"""Mailles volumiques, groupes noeuds et edges de fond de fissure, groupe de face de fissure"""

def construitMaillagePipe_d(idisk, \
                            idisklim, nbsegCercle, nbsegRad, \
                            meshPipe, mptdsk, oldmpts, \
                            fondFissGroup, edgeFaceFissGroup, faceFissGroup, \
                            mEdges, mEdgeFaces, mFaces, mVols, nodesFondFissGroup):
  """Mailles volumiques, groupes noeuds et edges de fond de fissure, groupe de face de fissure"""
  #logging.info('start')

  if idisk == idisklim[0]:
    mEdges.append(0)
    mEdgeFaces.append(0)
    mFaces.append([0])
    mVols.append([[0]])
    nodesFondFissGroup.Add([mptdsk[0][0]])

  else:
    ide = meshPipe.AddEdge([oldmpts[0][0], mptdsk[0][0]])
    mEdges.append(ide)
    fondFissGroup.Add([ide])
    nodesFondFissGroup.Add([mptdsk[0][0]])
    ide2 = meshPipe.AddEdge([oldmpts[0][-1], mptdsk[0][-1]])
    mEdgeFaces.append(ide2)
    edgeFaceFissGroup.Add([ide2])

    id_faces = list()
    id_volumes = list()

    for n_srad in range(nbsegRad):
      idf = meshPipe.AddFace([oldmpts[0][n_srad], mptdsk[0][n_srad], mptdsk[0][n_srad+1], oldmpts[0][n_srad+1]])
      faceFissGroup.Add([idf])
      id_faces.append(idf)

      id_vol_cercle = list()
      for n_cercle in range(nbsegCercle):
        n_cercle_1 = n_cercle+1
        if n_cercle ==  nbsegCercle-1:
          n_cercle_1 = 0
        if n_srad == 0:
          idv = meshPipe.AddVolume([mptdsk[n_cercle][n_srad], mptdsk[n_cercle][n_srad+1], mptdsk[n_cercle_1][n_srad+1], \
                                    oldmpts[n_cercle][n_srad], oldmpts[n_cercle][n_srad+1], oldmpts[n_cercle_1][n_srad+1]])
        else:
          idv = meshPipe.AddVolume([mptdsk[n_cercle][n_srad], mptdsk[n_cercle][n_srad+1], mptdsk[n_cercle_1][n_srad+1], mptdsk[n_cercle_1][n_srad], \
                                    oldmpts[n_cercle][n_srad], oldmpts[n_cercle][n_srad+1], oldmpts[n_cercle_1][n_srad+1], oldmpts[n_cercle_1][n_srad]])
        id_vol_cercle.append(idv)
      id_volumes.append(id_vol_cercle)

    mFaces.append(id_faces)
    mVols.append(id_volumes)

  return
