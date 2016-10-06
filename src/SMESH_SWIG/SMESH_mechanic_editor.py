# -*- coding: utf-8 -*-
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
smesh.SetName(hypNbSeg, "NumberOfSegments_" + str(numberOfSegment))


print "-------------------------- MaxElementArea"

maxElementArea = 25

algo = mesh.Triangle()
hypArea25 = algo.MaxElementArea(maxElementArea)
print hypArea25.GetName()
print hypArea25.GetId()
print hypArea25.GetMaxElementArea()
smesh.SetName(hypArea25, "MaxElementArea_" + str(maxElementArea))


# Create submesh on sub_face1 - sub_face4
# ---------------------------------------

# Set 2D algorithm to submesh on sub_face1
algo = mesh.Quadrangle(sub_face1)
smesh.SetName(algo.GetSubMesh(), "SubMeshFace1")
submesh1 = algo.GetSubMesh()

# Set 2D algorithm to submesh on sub_face2
algo = mesh.Quadrangle(sub_face2)
smesh.SetName(algo.GetSubMesh(), "SubMeshFace2")
submesh2 = algo.GetSubMesh()

# Set 2D algorithm to submesh on sub_face3
algo = mesh.Quadrangle(sub_face3)
smesh.SetName(algo.GetSubMesh(), "SubMeshFace3")
submesh3 = algo.GetSubMesh()

# Set 2D algorithm to submesh on sub_face4
algo = mesh.Quadrangle(sub_face4)
smesh.SetName(algo.GetSubMesh(), "SubMeshFace4")
submesh4 = algo.GetSubMesh()


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


#1 cutting of quadrangles of the 'SubMeshFace2' submesh
mesh.SplitQuadObject(submesh2, 1)

#2 cutting of triangles of the group
FacesTriToQuad = [ 2391, 2824, 2825, 2826, 2827, 2828, 2832, 2833, 2834, 2835, 2836, 2837, 2838, 2839, 2841, 2844, 2845, 2847, 2854, 2861, 2863, 2922, 2923, 2924, 2925, 2926, 2927, 2928, 2929, 2930, 2931, 2932, 2933, 2934, 2935, 2936, 2937, 2938, 2940, 2941, 2946, 2951, 2970, 2971, 2972, 2973, 2974, 2975, 2976, 2977, 2978, 2979, 2980, 2981, 2982, 2983, 2984, 2985 ]
GroupTriToQuad = mesh.MakeGroupByIds("Group of faces (quad)", SMESH.FACE, FacesTriToQuad)
mesh.TriToQuadObject(GroupTriToQuad, SMESH.FT_AspectRatio , 1.57)

#3 extrusion of the group
point = SMESH.PointStruct(0, 0, 5)
vector = SMESH.DirStruct(point)
mesh.ExtrusionSweepObject(GroupTriToQuad, vector, 5)

#4 mirror object
mesh.Mirror([], SMESH.AxisStruct(0, 0, 0, 0, 0, 0), smesh.POINT, 0)

#5 mesh translation
point = SMESH.PointStruct(10, 10, 10)
vector = SMESH.DirStruct(point)
mesh.Translate([], vector, 0)

#6 mesh rotation
axisXYZ = SMESH.AxisStruct(0, 0, 0, 10, 10, 10)
angle180 =  180*3.141/180
mesh.Rotate([], axisXYZ, angle180, 0)

#7 group smoothing
FacesSmooth = [864, 933, 941, 950, 1005, 1013]
GroupSmooth = mesh.MakeGroupByIds("Group of faces (smooth)", SMESH.FACE, FacesSmooth)
mesh.SmoothObject(GroupSmooth, [], 20, 2, smesh.CENTROIDAL_SMOOTH)

#8 rotation sweep object
FacesRotate = [492, 493, 502, 503]
GroupRotate = mesh.MakeGroupByIds("Group of faces (rotate)", SMESH.FACE, FacesRotate)
angle45 =  45*3.141/180
axisXYZ = SMESH.AxisStruct(-38.3128, -73.3658, -133.321, -13.3402, -13.3265, 6.66632)
mesh.RotationSweepObject(GroupRotate, axisXYZ, angle45, 4, 1e-5)

#9 reorientation of the submesh1
mesh.ReorientObject(submesh1)

salome.sg.updateObjBrowser(True)
