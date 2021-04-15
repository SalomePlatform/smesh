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
"""Les points"""

from .geomsmesh import geompy

def construitMaillagePipe_a(idisk, \
                            gptsdisks, nbsegCercle, \
                            meshPipe, mptsdisks):
  """Les points"""
  #logging.info('start')

  gptdsk = gptsdisks[idisk]

  mptdsk = list() # vertices maillage d'un disque
  for n_seg in range(nbsegCercle):

    points = gptdsk[n_seg]
    mptids = list()
    for n_point, point in enumerate(points):
      if n_point == 0 and n_seg > 0:
        n_noeud = mptdsk[0][0]
      else:
        coords = geompy.PointCoordinates(point)
        n_noeud = meshPipe.AddNode(coords[0], coords[1], coords[2])
      mptids.append(n_noeud)
    mptdsk.append(mptids)

  mptsdisks.append(mptdsk)

  return mptdsk
