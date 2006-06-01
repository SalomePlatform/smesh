#  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#
#
#  File   : SMESH_withHole.py
#  Author : Lucien PIGNOLONI
#  Module : SMESH
#  $Header$

#-------------------------------------------------------------------------

import salome
import geompy

import StdMeshers

import SMESH

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

# ---- add a face sub shape in study to be meshed different
sub_face1 = SubFaceL[0]
name      = geompy.SubShapeName( sub_face1, mechanic )

Id_SubFace1 = geompy.addToStudyInFather( mechanic, sub_face1, name )

# ---- add a face sub shape in study to be meshed different
sub_face2 = SubFaceL[4]
name      = geompy.SubShapeName( sub_face2, mechanic )

Id_SubFace2 = geompy.addToStudyInFather( mechanic, sub_face2, name )

# ---- add a face sub shape in study to be meshed different
sub_face3 = SubFaceL[5]
name      = geompy.SubShapeName( sub_face3, mechanic )

Id_SubFace3 = geompy.addToStudyInFather( mechanic, sub_face3, name )

# ---- add a face sub shape in study to be meshed different
sub_face4 = SubFaceL[10]
name      = geompy.SubShapeName( sub_face4, mechanic )

Id_SubFace4 = geompy.addToStudyInFather( mechanic, sub_face4, name )

# ---------------------------- SMESH --------------------------------------

# ---- launch SMESH, init a Mesh with shape 'mechanic'

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

# -- Init --
shape_mesh = salome.IDToObject( Id_mechanic )
smesh.SetCurrentStudy(salome.myStudy)

mesh = smesh.CreateMesh(shape_mesh)

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

idmesh = salome.ObjectToID(mesh)
smeshgui.SetName( idmesh, "Mesh_mechanic" )

print "-------------------------- NumberOfSegments"

numberOfSegment = 10

hypNbSeg = smesh.CreateHypothesis( "NumberOfSegments", "libStdMeshersEngine.so" )
hypNbSeg.SetNumberOfSegments( numberOfSegment )
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()

smeshgui.SetName(salome.ObjectToID(hypNbSeg), "NumberOfSegments_10")

print "-------------------------- MaxElementArea"

maxElementArea = 25

hypArea25 = smesh.CreateHypothesis( "MaxElementArea", "libStdMeshersEngine.so" )
hypArea25.SetMaxElementArea( maxElementArea )
print hypArea25.GetName()
print hypArea25.GetId()
print hypArea25.GetMaxElementArea()

smeshgui.SetName(salome.ObjectToID(hypArea25), "MaxElementArea_25")

print "-------------------------- MaxElementArea"

maxElementArea = 35

hypArea35 = smesh.CreateHypothesis( "MaxElementArea", "libStdMeshersEngine.so" )
hypArea35.SetMaxElementArea( maxElementArea )
print hypArea35.GetName()
print hypArea35.GetId()
print hypArea35.GetMaxElementArea()

smeshgui.SetName(salome.ObjectToID(hypArea35), "MaxElementArea_35")

print "-------------------------- Regular_1D"

algoReg1D = smesh.CreateHypothesis( "Regular_1D", "libStdMeshersEngine.so" )
listHyp = algoReg1D.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algoReg1D.GetName()
print algoReg1D.GetId()

smeshgui.SetName(salome.ObjectToID(algoReg1D), "Regular_1D")

print "-------------------------- MEFISTO_2D"

algoMef = smesh.CreateHypothesis( "MEFISTO_2D", "libStdMeshersEngine.so" )
listHyp = algoMef.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algoMef.GetName()
print algoMef.GetId()

smeshgui.SetName(salome.ObjectToID(algoMef), "MEFISTO_2D")

print "-------------------------- SMESH_Quadrangle_2D"

algoQuad = smesh.CreateHypothesis( "Quadrangle_2D", "libStdMeshersEngine.so" )
listHyp = algoQuad.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algoQuad.GetName()
print algoQuad.GetId()

smeshgui.SetName(salome.ObjectToID(algoQuad), "SMESH_Quadrangle_2D")

print "-------------------------- add hypothesis to main shape"

mesh.AddHypothesis( shape_mesh, hypNbSeg )   # nb segments
mesh.AddHypothesis( shape_mesh, hypArea25 )  # max area

mesh.AddHypothesis( shape_mesh, algoReg1D )  # Regular 1D/wire discretisation
mesh.AddHypothesis( shape_mesh, algoMef )    # MEFISTO 2D

print "-------------------------- add hypothesis and algorithm to sub face 1"

submesh = mesh.GetSubMesh(sub_face1, "SubMeshFace1")

mesh.AddHypothesis( sub_face1, algoQuad )   # Quadrangle 2D
mesh.AddHypothesis( sub_face1, hypArea35 )  # max area

print "-------------------------- add hypothesis and algorithm to sub face 2"

submesh = mesh.GetSubMesh(sub_face2, "SubMeshFace2")

mesh.AddHypothesis( sub_face2, algoQuad )   # Quadrangle 2D
mesh.AddHypothesis( sub_face2, hypArea35 )  # max area

print "-------------------------- add hypothesis and algorith to sub face 3"

submesh = mesh.GetSubMesh(sub_face3, "SubMeshFace3")

mesh.AddHypothesis( sub_face3, algoQuad )   # Quadrangle 2D
mesh.AddHypothesis( sub_face3, hypArea35 )  # max area

print "-------------------------- add hypothesis and algorith to sub face 4"

submesh = mesh.GetSubMesh(sub_face4, "SubMeshFace4")

mesh.AddHypothesis( sub_face4, algoQuad )   # Quadrangle 2D
mesh.AddHypothesis( sub_face4, hypArea35 )  # max area

print "-------------------------- compute the mesh of the mechanic piece"

smesh.Compute(mesh, shape_mesh)

print "Information about the Mesh_mechanic:"
print "Number of nodes       : ", mesh.NbNodes()
print "Number of edges       : ", mesh.NbEdges()
print "Number of faces       : ", mesh.NbFaces()
print "Number of triangles   : ", mesh.NbTriangles()
print "Number of quadrangles : ", mesh.NbQuadrangles()
print "Number of volumes     : ", mesh.NbVolumes()
print "Number of tetrahedrons: ", mesh.NbTetras()


MeshEditor = mesh.GetMeshEditor()

#1 cutting of quadrangles of the 'SubMeshFace2' submesh
submesh = mesh.GetSubMesh(sub_face2, "SubMeshFace2")
MeshEditor.SplitQuadObject(submesh, 1)

#2 cutting of triangles of the group
FacesTriToQuad = [2381, 2382, 2383, 2384, 2385, 2386, 2387, 2388, 2389, 2390, 2391, 2392, 2393, 2394, 2395, 2396, 2397, 2398, 2399, 2400, 2401, 2402, 2403, 2404, 2405, 2406, 2407, 2408, 2409, 2410, 2411, 2412, 2413, 2414, 2415, 2416, 2417, 2418, 2419, 2420, 2421, 2422]
GroupTriToQuad = mesh.CreateGroup(SMESH.FACE,"Group of faces (quad)")
GroupTriToQuad.Add(FacesTriToQuad)
MeshEditor.TriToQuadObject(GroupTriToQuad, None , 1.57)

#3 extrusion of the group
point = SMESH.PointStruct(0, 0, 5)
vector = SMESH.DirStruct(point) 
MeshEditor.ExtrusionSweepObject(GroupTriToQuad, vector, 5)

#4 mirror object
MeshEditor.MirrorObject(mesh, SMESH.AxisStruct(0, 0, 0, 0, 0, 0), SMESH.SMESH_MeshEditor.POINT, 0) 

#5 mesh translation
point = SMESH.PointStruct(10, 10, 10)
vector = SMESH.DirStruct(point) 
MeshEditor.TranslateObject(mesh, vector, 0)

#6 mesh rotation
axisXYZ = SMESH.AxisStruct(0, 0, 0, 10, 10, 10)
angle180 =  180*3.141/180
MeshEditor.RotateObject(mesh, axisXYZ, angle180, 0)

#7 group smoothing
FacesSmooth = [864, 933, 941, 950, 1005, 1013]
GroupSmooth = mesh.CreateGroup(SMESH.FACE,"Group of faces (smooth)")
GroupSmooth.Add(FacesSmooth)
MeshEditor.SmoothObject(GroupSmooth, [], 20, 2, SMESH.SMESH_MeshEditor.CENTROIDAL_SMOOTH)

#8 rotation sweep object
FacesRotate = [492, 493, 502, 503]
GroupRotate = mesh.CreateGroup(SMESH.FACE,"Group of faces (rotate)")
GroupRotate.Add(FacesRotate)
angle45 =  45*3.141/180
axisXYZ = SMESH.AxisStruct(-38.3128, -73.3658, -133.321, -13.3402, -13.3265, 6.66632)
MeshEditor.RotationSweepObject(GroupRotate, axisXYZ, angle45, 4, 1e-5)

#9 reorientation of the whole mesh
submesh = mesh.GetSubMesh(sub_face1, "SubMeshFace1")
MeshEditor.ReorientObject(submesh)

salome.sg.updateObjBrowser(1)
