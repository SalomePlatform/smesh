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
#  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
#
#
#
#  File   : SMESH_withHole.py
#  Author : Lucien PIGNOLONI
#  Module : SMESH
#  $Header$

import salome
import geompy

geom  = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

geom.GetCurrentStudy(salome.myStudy._get_StudyId())
smesh.SetCurrentStudy(salome.myStudy)

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);

import StdMeshers

# ---------------------------- GEOM --------------------------------------
ShapeTypeCompSolid = 1
ShapeTypeSolid     = 2
ShapeTypeShell     = 3
ShapeTypeFace      = 4
ShapeTypeWire      = 5
ShapeTypeEdge      = 6
ShapeTypeVertex    = 7

# ---- define contigous arcs and segment to define a closed wire
p1   = geom.MakePointStruct( 100.0,   0.0,  0.0 )
p2   = geom.MakePointStruct(  50.0,  50.0,  0.0 )
p3   = geom.MakePointStruct( 100.0, 100.0,  0.0 ) 
arc1 = geom.MakeArc( p1, p2, p3 )

p4   = geom.MakePointStruct( 170.0, 100.0, 0.0 )
seg1 = geom.MakeVector( p3, p4 )

p5   = geom.MakePointStruct( 200.0, 70.0, 0.0 )
p6   = geom.MakePointStruct( 170.0, 40.0, 0.0 )
arc2 = geom.MakeArc( p4, p5, p6 )

p7   = geom.MakePointStruct( 120.0, 30.0, 0.0 )
arc3 = geom.MakeArc( p6, p7, p1 )

# ---- define a closed wire with arcs and segment
List1 = []
List1.append( arc1 )
List1.append( seg1 )
List1.append( arc2 )
List1.append( arc3 )

ListIOR1 = []
for S in List1 :
    ListIOR1.append( S._get_Name() )
wire1 = geom.MakeWire( ListIOR1 )

Id_wire1 = geompy.addToStudy( wire1, "wire1")

# ---- define a planar face with wire
WantPlanarFace = 1 #True
face1 = geom.MakeFace( wire1, WantPlanarFace )
Id_face1 = geompy.addToStudy( face1, "face1")

# ---- create a shape by extrusion
pO = geom.MakePointStruct( 0.0, 0.0,   0.0 )
pz = geom.MakePointStruct( 0.0, 0.0, 100.0 )

prism1    = geom.MakePrism( face1, pO, pz )
Id_prism1 = geompy.addToStudy( prism1, "prism1")

# ---- create two cylinders

pc1 = geom.MakePointStruct(  90.0, 50.0, -40.0 )
pc2 = geom.MakePointStruct( 170.0, 70.0, -40.0 )
vz  = geom.MakeDirection( pz )
radius = 20.0
height = 180.0
cyl1  = geom.MakeCylinder( pc1, vz, radius, height )
cyl2  = geom.MakeCylinder( pc2, vz, radius, height )

Id_Cyl1 = geompy.addToStudy( cyl1, "cyl1" )
Id_Cyl2 = geompy.addToStudy( cyl2, "cyl2" )

# ---- cut with cyl1 
shape  = geom.MakeBoolean( prism1, cyl1, 2 )

# ---- fuse with cyl2 
shape1 =  geom.MakeBoolean( shape, cyl2, 3 )

Id_shape1 = geompy.addToStudy( shape1, "shape1")

#faces = geompy.SubShapeAllSorted( shape1, ShapeTypeFace)
#i = 0
#for face in faces:
#    geompy.addToStudy(face,"face_" + str(i))
#    i = i+1
    
# ---- add a face sub shape in study to be meshed different
IdSubFaceList = []
IdSubFaceList.append(1)
sub_face = geompy.SubShapeSorted( shape1, ShapeTypeFace, IdSubFaceList )
name     = geompy.SubShapeName( sub_face._get_Name(), shape1._get_Name() )

Id_SubFace = geompy.addToStudyInFather( shape1, sub_face, name )

# ---- add a face sub shape in study to be meshed different
IdSubFaceL = []
IdSubFaceL.append(2)
sub_face2 = geompy.SubShapeSorted( shape1, ShapeTypeFace, IdSubFaceL )
name      = geompy.SubShapeName( sub_face2._get_Name(), shape1._get_Name() )

Id_SubFace2 = geompy.addToStudyInFather( shape1, sub_face2, name )

# ---- add a face sub shape in study to be meshed different
IdSubFaceL = []
IdSubFaceL.append(3)
sub_face3 = geompy.SubShapeSorted( shape1, ShapeTypeFace, IdSubFaceL )
name      = geompy.SubShapeName( sub_face3._get_Name(), shape1._get_Name() )

Id_SubFace3 = geompy.addToStudyInFather( shape1, sub_face3, name )

# ---- add a face sub shape in study to be meshed different
IdSubFaceL = []
IdSubFaceL.append(6)
sub_face4 = geompy.SubShapeSorted( shape1, ShapeTypeFace, IdSubFaceL )
name      = geompy.SubShapeName( sub_face4._get_Name(), shape1._get_Name() )

Id_SubFace4 = geompy.addToStudyInFather( shape1, sub_face4, name )


# ---------------------------- SMESH --------------------------------------

# ------------------------------ Length Hypothesis

print "-------------------------- create Hypothesis"
print "-------------------------- LocalLength"
hyp1 = smesh.CreateHypothesis("LocalLength", "libStdMeshersEngine.so")
hyp1.SetLength( 100.0 )
print hyp1.GetName()
print hyp1.GetId()
print hyp1.GetLength()

idlength = salome.ObjectToID(hyp1)
smeshgui.SetName(idlength, "Local_Length_100");

print "-------------------------- NumberOfSegments"

hyp2 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hyp2.SetNumberOfSegments( 10 )
print hyp2.GetName()
print hyp2.GetId()
print hyp2.GetNumberOfSegments()

idseg = salome.ObjectToID(hyp2)
smeshgui.SetName(idseg, "NumberOfSegments_12");

print "-------------------------- MaxElementArea"

hyp3 = smesh.CreateHypothesis( "MaxElementArea", "libStdMeshersEngine.so" )
hyp3.SetMaxElementArea( 25 )
print hyp3.GetName()
print hyp3.GetId()
print hyp3.GetMaxElementArea()

idarea1 = salome.ObjectToID(hyp3)
smeshgui.SetName(idarea1, "MaxElementArea_20");

print "-------------------------- MaxElementArea"

hyp4 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp4.SetMaxElementArea( 35 )
print hyp4.GetName()
print hyp4.GetId()
print hyp4.GetMaxElementArea()

idarea2 = salome.ObjectToID(hyp4)
smeshgui.SetName(idarea2, "MaxElementArea_30");


print "-------------------------- Regular_1D"

alg1 = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
listHyp =alg1.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print alg1.GetName()
print alg1.GetId()

idreg1d = salome.ObjectToID(alg1)
smeshgui.SetName( idreg1d, "Regular_1D" );

print "-------------------------- MEFISTO_2D"

alg2 = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")
listHyp = alg2.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print alg2.GetName()
print alg2.GetId()

idmef = salome.ObjectToID(alg2)
smeshgui.SetName( idmef, "MEFISTO_2D" );

print "-------------------------- SMESH_Quadrangle_2D"

alg3 = smesh.CreateHypothesis( "Quadrangle_2D", "libStdMeshersEngine.so" )
listHyp = alg3.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print alg3.GetName()
print alg3.GetId()

idqad2 =  salome.ObjectToID(alg3)
smeshgui.SetName( idqad2, "SMESH_Quadrangle_2D" );


print "-------------------------- add hypothesis to main shape1"

shape_mesh = salome.IDToObject( Id_shape1  )

mesh = smesh.CreateMesh(shape_mesh)

idmesh = salome.ObjectToID(mesh) 
smeshgui.SetName( idmesh, "Mesh_mechanic" );

mesh.AddHypothesis( shape_mesh, alg1 )   # Regular 1D/wire discretisation
mesh.AddHypothesis( shape_mesh, alg2 )     # MEFISTO 2D

ret = mesh.AddHypothesis( shape_mesh, hyp2 )   # nb segments
ret = mesh.AddHypothesis( shape_mesh, hyp3 )    # max area

print "--------Add hypothesis and algorith to sub face"        

#sub_face  = salome.IDToObject( Id_SubFace )
submesh   = mesh.GetSubMesh(sub_face, "SubMeshFace")

mesh.AddHypothesis( sub_face, alg3 )  # Quadrangle 2D
mesh.AddHypothesis( sub_face, hyp4 )  # max area

print "--------Add hypothesis and algorith to sub face 2"        

#sub_face2  = salome.IDToObject( Id_SubFace2 )
submesh    = mesh.GetSubMesh(sub_face2, "SubMeshFace2")

mesh.AddHypothesis( sub_face2, alg3 )  # Quadrangle 2D
ret = mesh.AddHypothesis( sub_face2, hyp4 )  # max area

print "--------Add hypothesis and algorith to sub face 3"        

#sub_face3  = salome.IDToObject( Id_SubFace3 )
submesh    = mesh.GetSubMesh(sub_face3, "SubMeshFace3")

mesh.AddHypothesis( sub_face3, alg3 )  # Quadrangle 2D
ret = mesh.AddHypothesis( sub_face3, hyp4 )  # max area

print "--------Add hypothesis and algorith to sub face 4"        

#sub_face4  = salome.IDToObject( Id_SubFace4 )
submesh    = mesh.GetSubMesh(sub_face4, "SubMeshFace4")

mesh.AddHypothesis( sub_face4, alg3 )  # Quadrangle 2D
ret = mesh.AddHypothesis( sub_face4, hyp4 )  # max area

smesh.Compute(mesh,shape_mesh)

print "Information about the Mesh_mechanic:"
print "Number of nodes      : ", mesh.NbNodes()
print "Number of edges      : ", mesh.NbEdges()
print "Number of faces      : ", mesh.NbFaces()
print "Number of triangles  : ", mesh.NbTriangles()
print "Number of quadrangles: ", mesh.NbQuadrangles()

salome.sg.updateObjBrowser(1);
