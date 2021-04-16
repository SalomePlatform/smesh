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

dicoParams = dict(nomCas            = "cubeTransverse",
                  nomProbleme       = "cubeTransverse",
                  maillageSain      = os.path.join(gmu.pathBloc, "materielCasTests", "cubeFin.med"),
                  CAOFaceFissure    = os.path.join(gmu.pathBloc, "materielCasTests", "cubeFin_Transverse.brep"),
                  edgeFiss          = [6],
                  lgInfluence       = 50,
                  meshBrep          = (5,10),
                  rayonPipe         = 10,
                  lenSegPipe        = 7,
                  nbSegRad          = 5,
                  nbSegCercle       = 10,
                  areteFaceFissure  = 10)

# ---------------------------------------------------------------------------

referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 2900, \
                                          Entity_Quad_Hexa = 7000, \
                                          Entity_Node = 58430, \
                                          Entity_Quad_Edge = 390, \
                                          Entity_Quad_Triangle = 1228, \
                                          Entity_Quad_Tetra = 17567, \
                                          Entity_Quad_Pyramid = 780, \
                                          Entity_Quad_Penta = 200 \
                                )
