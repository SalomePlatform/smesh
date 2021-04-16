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

"""problème de fissure plane coupant le cube au milieu"""

import os
from blocFissure import gmu

dicoParams = dict(nomCas            = "cubeMilieu",
                  nomProbleme       = "cubeMilieu",
                  maillageSain      = os.path.join(gmu.pathBloc, "materielCasTests", "cubeFin.med"),
                  CAOFaceFissure    = os.path.join(gmu.pathBloc, "materielCasTests", "cubeFin_Milieu.brep"),
                  edgeFiss          = [3],
                  lgInfluence       = 50,
                  meshBrep          = (5,10),
                  rayonPipe         = 10,
                  lenSegPipe        = 7,
                  nbSegRad          = 5,
                  nbSegCercle       = 10,
                  areteFaceFissure  = 10)

# ---------------------------------------------------------------------------

referencesMaillageFissure = dict( \
                                Entity_Quad_Quadrangle = 3140, \
                                Entity_Quad_Hexa = 7120, \
                                Entity_Node = 61414, \
                                Entity_Quad_Edge = 346, \
                                Entity_Quad_Triangle = 818, \
                                Entity_Quad_Tetra = 19117, \
                                Entity_Quad_Pyramid = 990, \
                                Entity_Quad_Penta = 230 \
                                )
