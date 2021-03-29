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

import sys
import salome

salome.salome_init()

import salome_notebook

import os
from blocFissure import gmu

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS


geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Vertex_1 = geompy.MakeVertex(100, 0, 0)
Disk_1 = geompy.MakeDiskPntVecR(Vertex_1, OY, 12.5)
Scale_1 = geompy.MakeScaleAlongAxes(Disk_1, Vertex_1, 1, 1, 4.1)
Vertex_2 = geompy.MakeVertex(98, -2, -2)
Vertex_3 = geompy.MakeVertex(120, 2, 60)
Box_1 = geompy.MakeBoxTwoPnt(Vertex_3, Vertex_2)
Ellipse_disque = geompy.MakeCommon(Box_1, Scale_1)
geompy.ExportBREP(Ellipse_disque, os.path.join(gmu.pathBloc, "materielCasTests", "ellipse_disque.brep"))
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Scale_1, 'Scale_1' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Ellipse_disque, 'Ellipse_disque' )


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
