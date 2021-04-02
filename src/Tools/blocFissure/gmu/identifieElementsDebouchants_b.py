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
"""Estimation de la longueur du pipe necessaire de part et d'autre du point de sortie"""

import logging
import math

from .geomsmesh import geompy

from .whichSide import whichSide

def identifieElementsDebouchants_b(ifil, \
                                  facesDefaut,aretesVivesC, fillingFaceExterne, rayonPipe, \
                                  ptPeau, centre, norm, localEdgeInFondFiss):
  """Estimation de la longueur du pipe necessaire de part et d'autre du point de sortie"""

  logging.info('start')

  # --- estimation de la longueur du pipe necessaire de part et d'autre du point de sortie
  if aretesVivesC is None:
    face_test_peau = fillingFaceExterne
  else:
    face_test_peau = facesDefaut[ifil]
  side_centre = whichSide(face_test_peau, centre)
  loc_pt0 = geompy.MakeVertexOnCurve(localEdgeInFondFiss, 0.0)
  loc_pt1 = geompy.MakeVertexOnCurve(localEdgeInFondFiss, 1.0)
  side_point_0 = whichSide(face_test_peau, loc_pt0)
  side_point_1 = whichSide(face_test_peau, loc_pt1)
  logging.debug("position centre cercle: %s, extremité edge u0: %s, u1: %s", side_centre, side_point_0, side_point_1)
  norm_face = geompy.GetNormal(face_test_peau, ptPeau)
  incl_pipe = abs(geompy.GetAngleRadians(norm, norm_face))
  lgp = max(rayonPipe/2., abs(3*rayonPipe*math.tan(incl_pipe)))
  logging.debug("angle inclinaison Pipe en sortie: %s degres, lgp: %s", incl_pipe*180/math.pi, lgp)

  return loc_pt0, lgp
