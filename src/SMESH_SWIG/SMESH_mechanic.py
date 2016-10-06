#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#  File   : SMESH_withHole.py
#  Author : Lucien PIGNOLONI
#  Module : SMESH
#  $Header$
#-------------------------------------------------------------------------
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

from salome.StdMeshers import StdMeshersBuilder

# ---------------------------- GEOM --------------------------------------

# ---- define contigous arcs and segment to define a closed wire
p1   = geompy.MakeVertex( 100.0,   0.0,  0.0 )
p2   = geompy.MakeVertex(  50.0,  50.0,  0.0 )
p3   = geompy.MakeVertex( 100.0, 100.0,  0.0 )
arc1 = geompy.MakeArc( p1, p2, p3 )

p4   = geompy.MakeVertex( 170.0, 100.0, 0.0 )
seg1 = geompy.MakeVector( p3, p4 )

p5   = geompy.MakeVertex( 200.0, 70.0, 0.0 )
p6   = geompy.MakeVertex( 170.0, 40.0, 0.0 )
arc2 = geompy.MakeArc( p4, p5, p6 )

p7   = geompy.MakeVertex( 120.0, 30.0, 0.0 )
arc3 = geompy.MakeArc( p6, p7, p1 )

# ---- define a closed wire with arcs and segment
List1 = []
List1.append( arc1 )
List1.append( seg1 )
List1.append( arc2 )
List1.append( arc3 )

wire1 = geompy.MakeWire( List1 )
Id_wire1 = geompy.addToStudy( wire1, "wire1" )

# ---- define a planar face with wire
WantPlanarFace = 1 #True
face1 = geompy.MakeFace( wire1, WantPlanarFace )
Id_face1 = geompy.addToStudy( face1, "face1" )

# ---- create a shape by extrusion
pO = geompy.MakeVertex( 0.0, 0.0,   0.0 )
pz = geompy.MakeVertex( 0.0, 0.0, 100.0 )
vz = geompy.MakeVector( pO, pz )

prism1 = geompy.MakePrismVecH( face1, vz, 100.0 )
Id_prism1 = geompy.addToStudy( prism1, "prism1" )

# ---- create two cylinders
pc1 = geompy.MakeVertex(  90.0, 50.0, -40.0 )
pc2 = geompy.MakeVertex( 170.0, 70.0, -40.0 )

radius = 20.0
height = 180.0
cyl1 = geompy.MakeCylinder( pc1, vz, radius, height )
cyl2 = geompy.MakeCylinder( pc2, vz, radius, height )

Id_Cyl1 = geompy.addToStudy( cyl1, "cyl1" )
Id_Cyl2 = geompy.addToStudy( cyl2, "cyl2" )

# ---- cut with cyl1
shape = geompy.MakeBoolean( prism1, cyl1, 2 )

# ---- fuse with cyl2 to obtain the final mechanic piece :)
mechanic = geompy.MakeBoolean( shape, cyl2, 3 )
Id_mechanic = geompy.addToStudy( mechanic, "mechanic" )

# ---- explode on faces
SubFaceL = geompy.SubShapeAllSorted(mechanic, geompy.ShapeType["FACE"])

# ---- add a face sub-shape in study to be meshed different
sub_face1 = SubFaceL[0]
name      = geompy.SubShapeName( sub_face1, mechanic )

Id_SubFace1 = geompy.addToStudyInFather( mechanic, sub_face1, name )

# ---- add a face sub-shape in study to be meshed different
sub_face2 = SubFaceL[4]
name      = geompy.SubShapeName( sub_face2, mechanic )

Id_SubFace2 = geompy.addToStudyInFather( mechanic, sub_face2, name )

# ---- add a face sub-shape in study to be meshed different
sub_face3 = SubFaceL[5]
name      = geompy.SubShapeName( sub_face3, mechanic )

Id_SubFace3 = geompy.addToStudyInFather( mechanic, sub_face3, name )

# ---- add a face sub-shape in study to be meshed different
sub_face4 = SubFaceL[10]
name      = geompy.SubShapeName( sub_face4, mechanic )

Id_SubFace4 = geompy.addToStudyInFather( mechanic, sub_face4, name )

# ---------------------------- SMESH --------------------------------------
smesh.SetCurrentStudy(salome.myStudy)

# -- Init --
shape_mesh = salome.IDToObject( Id_mechanic )

mesh = smesh.Mesh(shape_mesh, "Mesh_mechanic")

print "-------------------------- NumberOfSegments"

numberOfSegment = 10

algo = mesh.Segment()
hypNbSeg = algo.NumberOfSegments(numberOfSegment)
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()
smesh.SetName(hypNbSeg, "NumberOfSegments_10")

print "-------------------------- MaxElementArea"

maxElementArea = 25

algo = mesh.Triangle()
hypArea25 = algo.MaxElementArea(maxElementArea)
print hypArea25.GetName()
print hypArea25.GetId()
print hypArea25.GetMaxElementArea()
smesh.SetName(hypArea25, "MaxElementArea_25")

# Create submesh on sub_face1 - sub_face4
# ---------------------------------------

# Set 2D algorithm to submesh on sub_face1
algo = mesh.Quadrangle(sub_face1)
smesh.SetName(algo.GetSubMesh(), "SubMeshFace1")

# Set 2D algorithm to submesh on sub_face2
algo = mesh.Quadrangle(sub_face2)
smesh.SetName(algo.GetSubMesh(), "SubMeshFace2")

# Set 2D algorithm to submesh on sub_face3
algo = mesh.Quadrangle(sub_face3)
smesh.SetName(algo.GetSubMesh(), "SubMeshFace3")

# Set 2D algorithm to submesh on sub_face4
algo = mesh.Quadrangle(sub_face4)
smesh.SetName(algo.GetSubMesh(), "SubMeshFace4")

print "-------------------------- compute the mesh of the mechanic piece"

mesh.Compute()

print "Information about the Mesh_mechanic:"
print "Number of nodes       : ", mesh.NbNodes()
print "Number of edges       : ", mesh.NbEdges()
print "Number of faces       : ", mesh.NbFaces()
print "Number of triangles   : ", mesh.NbTriangles()
print "Number of quadrangles : ", mesh.NbQuadrangles()
print "Number of volumes     : ", mesh.NbVolumes()
print "Number of tetrahedrons: ", mesh.NbTetras()

salome.sg.updateObjBrowser(True)
