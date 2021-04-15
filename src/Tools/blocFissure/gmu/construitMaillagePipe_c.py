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
"""Les groupes des faces débouchantes"""

import SMESH

def construitMaillagePipe_c(idisk, \
                            idisklim, nbsegCercle, \
                            meshPipe, mptdsk, nbsegRad):
  """Les groupes des faces débouchantes"""
  #logging.info('start')

  faces = list()
  for n_seg in range(nbsegRad):
    for n_cercle in range(nbsegCercle):
      n_cercle_1 = n_cercle+1
      if n_cercle ==  nbsegCercle-1:
        n_cercle_1 = 0
      if n_seg == 0:
        id_face = meshPipe.AddFace([mptdsk[n_cercle][0], mptdsk[n_cercle][1], mptdsk[n_cercle_1][1]]) # triangle
      else:
        id_face = meshPipe.AddFace([mptdsk[n_cercle][n_seg], mptdsk[n_cercle][n_seg+1], mptdsk[n_cercle_1][n_seg+1], mptdsk[n_cercle_1][n_seg]]) # quadrangle
      faces.append(id_face)

  if idisk == idisklim[0]:
    groupe = meshPipe.CreateEmptyGroup(SMESH.FACE, "faceCircPipe0")
  else:
    groupe = meshPipe.CreateEmptyGroup(SMESH.FACE, "faceCircPipe1")

  groupe.Add(faces)

  return
