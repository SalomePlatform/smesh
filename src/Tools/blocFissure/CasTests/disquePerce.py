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

"""disque percé"""

import os
from blocFissure import gmu

dicoParams = dict(nomCas            = 'disquePerce',
                  nomProbleme       = "disquePerce",
                  maillageSain      = os.path.join(gmu.pathBloc, "materielCasTests", "disque.med"),
                  CAOFaceFissure    = os.path.join(gmu.pathBloc, "materielCasTests", "ellipse_disque.brep"),
                  edgeFiss          = [3],
                  lgInfluence       = 10,
                  meshBrep          = (0.5,2.5),
                  rayonPipe         = 1.0,
                  lenSegPipe        = 1.5,
                  nbSegRad          = 6,
                  nbSegCercle       = 16,
                  areteFaceFissure  = 2.5)

  # ---------------------------------------------------------------------------

referencesMaillageFissure = dict ( \
                                  Entity_Quad_Edge = 376, \
                                  Entity_Quad_Quadrangle = 2748, \
                                  Entity_Quad_Hexa = 6232, \
                                  Entity_Node = 43479, \
                                  Entity_Quad_Triangle = 1340, \
                                  Entity_Quad_Tetra = 8821, \
                                  Entity_Quad_Pyramid = 466, \
                                  Entity_Quad_Penta = 448 \
                                 )
