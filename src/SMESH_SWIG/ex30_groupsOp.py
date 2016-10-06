#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import math

global Face_1
Face_1 = geompy.MakeFaceHW(100, 100, 1)
geompy.addToStudy( Face_1, "Face_1" )

smesh.SetCurrentStudy(salome.myStudy)
from salome.StdMeshers import StdMeshersBuilder
pattern = smesh.GetPattern()
Mesh_1 = smesh.Mesh(Face_1)
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh_1.Quadrangle()
isDone = Mesh_1.Compute()

# groups creation

aListOfElems = [ 52, 53, 54, 55, 56, 57,
                 62, 63, 64, 65, 66, 67,
                 72, 73, 74, 75, 76, 77,
                 82, 83, 84, 85, 86, 87 ]
                 
aRedGroup = Mesh_1.GetMesh().CreateGroup( SMESH.FACE, "Red" )
aRedGroup.Add( aListOfElems );
aRedGroup.SetColor( SALOMEDS.Color( 1, 0, 0 ) )

aListOfElems = [ 55, 56, 57, 58, 59,
                 65, 66, 67, 68, 69,
                 75, 76, 77, 78, 79,
                 85, 86, 87, 88, 89,
                 95, 96, 97, 98, 99,
                 105, 106, 107, 108, 109,
                 115, 116, 117, 118, 119,
                 125, 126, 127, 128, 129 ]
                 
aGreenGroup = Mesh_1.GetMesh().CreateGroup( SMESH.FACE, "Green" )
aGreenGroup.Add( aListOfElems );
aGreenGroup.SetColor( SALOMEDS.Color( 0, 1, 0 ) )

aListOfElems = [ 63, 64, 65, 66, 67, 68, 
                 73, 74, 75, 76, 77, 78,
                 83, 84, 85, 86, 87, 88, 
                 93, 94, 95, 96, 97, 98, 
                 103, 104, 105, 106, 107, 108, 
                 113, 114, 115, 116, 117, 118 ]
                 
aBlueGroup = Mesh_1.GetMesh().CreateGroup( SMESH.FACE, "Blue" )
aBlueGroup.Add( aListOfElems );
aBlueGroup.SetColor( SALOMEDS.Color( 0, 0, 1 ) )

# UnionListOfGroups()
aUnGrp = Mesh_1.UnionListOfGroups([aRedGroup, aGreenGroup, aBlueGroup], "UnionGrp" )

# IntersectListOfGroups()
aIntGrp=Mesh_1.IntersectListOfGroups([aRedGroup, aGreenGroup, aBlueGroup], "IntGrp" )

# CutListOfGroups()
aCutGrp=Mesh_1.CutListOfGroups([aRedGroup],[aGreenGroup,aBlueGroup],"CutGrp")

salome.sg.updateObjBrowser(True)

