#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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
import geompy
import math
import SALOMEDS
import SMESH
import smesh

salome.salome_init()
aStudyId = salome.myStudy._get_StudyId()

geompy.init_geom(salome.myStudy)

geompy.init_geom(salome.myStudy)
global Box_1
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
geompy.addToStudy( Box_1, "Box_1" )

smesh.SetCurrentStudy(salome.myStudy)
import StdMeshers
Mesh_1 = smesh.Mesh(Box_1)
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh_1.Quadrangle()
Hexa_3D = Mesh_1.Hexahedron()
isDone = Mesh_1.Compute()

### CreateDimGroup()

aListOf3d_1=range(721,821)

aGrp3D_1=Mesh_1.GetMesh().CreateGroup( smesh.VOLUME, "Src 3D 1" )
aGrp3D_1.Add( aListOf3d_1 )

aListOf3d_2=range(821, 921)
aGrp3D_2=Mesh_1.GetMesh().CreateGroup( smesh.VOLUME, "Src 3D 2" )
aGrp3D_2.Add( aListOf3d_2 )

aGrp2D = Mesh_1.CreateDimGroup( [aGrp3D_1, aGrp3D_2], smesh.FACE, "Faces" )

aGrp1D = Mesh_1.CreateDimGroup( [aGrp3D_1, aGrp3D_2], smesh.EDGE, "Edges" )

aGrp0D = Mesh_1.CreateDimGroup( [aGrp3D_1, aGrp3D_2], smesh.NODE, "Nodes" )

salome.sg.updateObjBrowser( 1 )

