#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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

# =======================================
# Testing groups GMSH
#  File   : SMESH_GmshIOShapes.py
#  Module : SMESH

import sys
from salome.kernel import salome
import os
import tempfile

salome.salome_init()

###
### GEOM component
###

from salome.kernel import GEOM
from salome.geom import geomBuilder
import math
from salome.kernel import SALOMEDS

geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Group_Edges = geompy.CreateGroup(Box_1, geompy.ShapeType["EDGE"])
geompy.UnionIDs(Group_Edges, [18, 22])
Group_Faces = geompy.CreateGroup(Box_1, geompy.ShapeType["FACE"])
geompy.UnionIDs(Group_Faces, [3, 23])
Group_Vertices = geompy.CreateGroup(Box_1, geompy.ShapeType["VERTEX"])
geompy.UnionIDs(Group_Vertices, [6, 16])
Group_Volume = geompy.CreateGroup(Box_1, geompy.ShapeType["SOLID"])
geompy.UnionIDs(Group_Volume, [1])
[Group_Edges, Group_Faces, Group_Vertices, Group_Volume] = geompy.GetExistingSubObjects(Box_1, False)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudyInFather( Box_1, Group_Edges, 'Group_Edges' )
geompy.addToStudyInFather( Box_1, Group_Faces, 'Group_Faces' )
geompy.addToStudyInFather( Box_1, Group_Vertices, 'Group_Vertices' )
geompy.addToStudyInFather( Box_1, Group_Volume, 'Group_Volume' )

###
### SMESH component
###

from salome.kernel import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
#smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                 # multiples meshes built in parallel, complex and numerous mesh edition (performance)

Mesh_1 = smesh.Mesh(Box_1,'Mesh_1')
NETGEN_1D_2D_3D = Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
NETGEN_3D_Parameters_1 = NETGEN_1D_2D_3D.Parameters()
NETGEN_3D_Parameters_1.SetMaxSize( 34.641 )
NETGEN_3D_Parameters_1.SetMinSize( 0.34641 )
NETGEN_3D_Parameters_1.SetSecondOrder( 0 )
NETGEN_3D_Parameters_1.SetOptimize( 1 )
NETGEN_3D_Parameters_1.SetFineness( 2 )
NETGEN_3D_Parameters_1.SetChordalError( -1 )
NETGEN_3D_Parameters_1.SetChordalErrorEnabled( 0 )
NETGEN_3D_Parameters_1.SetUseSurfaceCurvature( 1 )
NETGEN_3D_Parameters_1.SetFuseEdges( 1 )
NETGEN_3D_Parameters_1.SetQuadAllowed( 0 )
NETGEN_3D_Parameters_1.SetCheckChartBoundary( 0 )
Group_Edges_1 = Mesh_1.GroupOnGeom(Group_Edges,'Group_Edges',SMESH.EDGE)
Group_Faces_1 = Mesh_1.GroupOnGeom(Group_Faces,'Group_Faces',SMESH.FACE)
Group_Vertices_1 = Mesh_1.GroupOnGeom(Group_Vertices,'Group_Vertices',SMESH.NODE)
Group_Volume_1 = Mesh_1.GroupOnGeom(Group_Volume,'Group_Volume',SMESH.VOLUME)
isDone = Mesh_1.Compute()
if not isDone:
  raise Exception('Mesh computation failed.')

Mesh_1.CheckCompute()
Group_0DElements = Mesh_1.Add0DElementsToAllNodes( [ 188, 195, 189 ], 'Group_0DElements' )
[ Group_Edges_1, Group_Faces_1, Group_Vertices_1, Group_Volume_1, Group_0DElements ] = Mesh_1.GetGroups()

## Set names of Mesh objects
smesh.SetName(Group_Vertices_1, 'Group_Vertices')
smesh.SetName(Group_0DElements, 'Group_0DElements')
smesh.SetName(Group_Volume_1, 'Group_Volume')
smesh.SetName(NETGEN_3D_Parameters_1, 'NETGEN 3D Parameters_1')
smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')
smesh.SetName(Group_Faces_1, 'Group_Faces')
smesh.SetName(Group_Edges_1, 'Group_Edges')
smesh.SetName(NETGEN_1D_2D_3D.GetAlgorithm(), 'NETGEN 1D-2D-3D')


# TODO: bug in GMSH about group of nodes - for now on, we skip the Group_Vertices
expected_group_names = ['Group_0DElements', 'Group_Volume', 'Group_Faces', 'Group_Edges'] # , 'Group_Vertices']

with tempfile.TemporaryDirectory() as tmp_dir:
    mesh_file = os.path.join(tmp_dir, "Mesh_1.msh")
    try:
      Mesh_1.ExportGMSHIO(mesh_file, 'Gmsh 4.1', Mesh_1)
    except:
      raise Exception("ExportPartToMESHIO() failed. Invalid file name?")
    # reimport file to ensure it can be read back
    ([Mesh_1_reimported], status) = smesh.CreateMeshesFromGMSHIO(mesh_file, "Gmsh 4.1")
    Mesh_1_groups = Mesh_1_reimported.GetGroups()
    group_names = [group.GetName().strip() for group in Mesh_1_groups]
    for gName in expected_group_names:
        assert gName in group_names

    # check that the group of edges has the correct number of elements
    assert Group_Edges_1.GetTypes() == [SMESH.EDGE]
    # check that the group of 0d Elements is present
    assert Group_0DElements.GetTypes() == [SMESH.ELEM0D]

    # Check with GMSH Python API if the .msh file contains the physical groups
    try:
        import gmsh
        gmsh.initialize()
        gmsh.open(mesh_file)
        model = gmsh.model
        physical_groups = model.getPhysicalGroups()
        physical_group_names = [model.getPhysicalName(dim, tag) for dim, tag in physical_groups]
        for gName in expected_group_names:
            assert gName in physical_group_names
        print("All expected physical groups are present in the .msh file.")
        gmsh.finalize()
    except ImportError:
        print("gmsh Python API is not available; skipping physical group check.")
if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
