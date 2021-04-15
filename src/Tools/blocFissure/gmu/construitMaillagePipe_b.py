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
"""Les groupes des edges des cercles débouchants"""

import SMESH

def construitMaillagePipe_b(idisk, \
                            idisklim, nbsegCercle, \
                            meshPipe, mptdsk, \
                            edgesCircPipeGroup):
  """Les groupes des edges des cercles débouchants"""
  #logging.info('start')

  pts = list()
  for n_cercle in range(nbsegCercle):
    pts.append(mptdsk[n_cercle][-1])

  edges = list()
  nb_pts = len(pts)
  for n_cercle in range(nb_pts):
    n_cercle_1 = (n_cercle+1)%nb_pts
    id_edge = meshPipe.AddEdge([pts[n_cercle], pts[n_cercle_1]])
    edges.append(id_edge)

  if idisk == idisklim[0]:
    groupe = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeCircPipe0")
  else:
    groupe = meshPipe.CreateEmptyGroup(SMESH.EDGE, "edgeCircPipe1")

  groupe.Add(edges)
  edgesCircPipeGroup.append(groupe)

  return
