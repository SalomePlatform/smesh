#==============================================================================
#  File      : SMESH_withHole.py
#  Created   : 26 august 2002
#  Author    : Lucien PIGNOLONI
#  Project   : SALOME
#  Copyright : Open CASCADE, 2002
#  $Header$
#==============================================================================

import SMESH
import smeshpy
import salome
from salome import sg
import math
#import SMESH_BasicHypothesis_idl

import geompy



# ---------------------------- GEOM --------------------------------------
geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
myBuilder = salome.myStudy.NewBuilder()
#from geompy import gg

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);

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


# ---- add a face sub shape in study to be meshed different
IdSubFaceList = []
IdSubFaceList.append(10)
sub_face = geompy.SubShapeSorted( shape1, ShapeTypeFace, IdSubFaceList )
name     = geompy.SubShapeName( sub_face._get_Name(), shape1._get_Name() )

Id_SubFace = geompy.addToStudyInFather( shape1, sub_face, name )


# ---- add a face sub shape in study to be meshed different
IdSubFaceL = []
IdSubFaceL.append(7)
sub_face2 = geompy.SubShapeSorted( shape1, ShapeTypeFace, IdSubFaceL )
name      = geompy.SubShapeName( sub_face2._get_Name(), shape1._get_Name() )

Id_SubFace2 = geompy.addToStudyInFather( shape1, sub_face2, name )




# ---------------------------- SMESH --------------------------------------

# ---- launch SMESH, init a Mesh with shape 'shape1'
gen  = smeshpy.smeshpy()
mesh = gen.Init( Id_shape1 )

idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName( idmesh, "Mesh_meca" );
smeshgui.SetShape( Id_shape1, idmesh );


# ------------------------------ Length Hypothesis

print "-------------------------- create Hypothesis"
print "-------------------------- LocalLength"
hyp1    = gen.CreateHypothesis( "LocalLength" )
hypLen1 = hyp1._narrow( SMESH.SMESH_LocalLength )
hypLen1.SetLength( 100.0 )
print hypLen1.GetName()
print hypLen1.GetId()
print hypLen1.GetLength()

idlength = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypLen1) );
smeshgui.SetName(idlength, "Local_Length_100");


print "-------------------------- NumberOfSegments"

hyp2      = gen.CreateHypothesis( "NumberOfSegments" )
hypNbSeg1 = hyp2._narrow( SMESH.SMESH_NumberOfSegments )
hypNbSeg1.SetNumberOfSegments( 10 )
print hypNbSeg1.GetName()
print hypNbSeg1.GetId()
print hypNbSeg1.GetNumberOfSegments()

idseg = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypNbSeg1) );
smeshgui.SetName(idseg, "NumberOfSegments_12");


print "-------------------------- MaxElementArea"

hyp3     = gen.CreateHypothesis( "MaxElementArea" )
hypArea1 = hyp3._narrow( SMESH.SMESH_MaxElementArea )
hypArea1.SetMaxElementArea( 25 )
print hypArea1.GetName()
print hypArea1.GetId()
print hypArea1.GetMaxElementArea()

idarea1 = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypArea1) );
smeshgui.SetName(idarea1, "MaxElementArea_20");



print "-------------------------- MaxElementArea"

hyp4     = gen.CreateHypothesis( "MaxElementArea" )
hypArea2 = hyp4._narrow( SMESH.SMESH_MaxElementArea )
hypArea2.SetMaxElementArea( 35 )
print hypArea2.	GetName()
print hypArea2.GetId()
print hypArea2.GetMaxElementArea()

idarea2 = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypArea2) );
smeshgui.SetName(idarea2, "MaxElementArea_30");


print "-------------------------- Regular_1D"

alg1    = gen.CreateHypothesis( "Regular_1D" )
algo1   = alg1._narrow( SMESH.SMESH_Algo )
listHyp =algo1.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
algoReg1d = alg1._narrow( SMESH.SMESH_Regular_1D )
print algoReg1d.GetName()
print algoReg1d.GetId()

idreg1d = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(algoReg1d) );
smeshgui.SetName( idreg1d, "Regular_1D" );



print "-------------------------- MEFISTO_2D"

alg2    = gen.CreateHypothesis( "MEFISTO_2D" )
algo2   = alg2._narrow( SMESH.SMESH_Algo )
listHyp = algo2.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
algoMef = alg2._narrow( SMESH.SMESH_MEFISTO_2D )
print algoMef.GetName()
print algoMef.GetId()

idmef = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(algoMef) );
smeshgui.SetName( idmef, "MEFISTO_2D" );



print "-------------------------- SMESH_Quadrangle_2D"

alg3    = gen.CreateHypothesis( "Quadrangle_2D" )
algo3   = alg3._narrow( SMESH.SMESH_2D_Algo )
listHyp = algo3.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
algoQad2 = alg3._narrow( SMESH.SMESH_Quadrangle_2D )
print algoQad2.GetName()
print algoQad2.GetId()

idqad2 = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(algoQad2) );
smeshgui.SetName( idqad2, "SMESH_Quadrangle_2D" );



print "-------------------------- add hypothesis to main shape1"

shape_mesh = salome.IDToObject( Id_shape1  )
submesh    = mesh.GetElementsOnShape( shape_mesh )

ret = mesh.AddHypothesis( shape_mesh, algoReg1d )   # Regular 1D/wire discretisation
print ret
ret = mesh.AddHypothesis( shape_mesh, algoMef )     # MEFISTO 2D
print ret
ret = mesh.AddHypothesis( shape_mesh, hypNbSeg1 )   # nb segments
print ret
ret = mesh.AddHypothesis( shape_mesh, hypArea1 )    # max area
print ret

smeshgui.SetAlgorithms( idmesh, idreg1d );  # Regular 1D/wire discretisation
smeshgui.SetAlgorithms( idmesh, idmef );    # MEFISTO 2D
smeshgui.SetHypothesis( idmesh, idseg );    # nb segments
smeshgui.SetHypothesis( idmesh, idarea1 );  # max area



print "-------------------------- add hypothesis and algorith to sub face"        

sub_face  = salome.IDToObject( Id_SubFace )
submesh   = mesh.GetElementsOnShape( sub_face )

ret = mesh.AddHypothesis( sub_face, algoQad2 )  # Quadrangle 2D
print ret
ret = mesh.AddHypothesis( sub_face, hypArea2 )  # max area
print ret

idsm2 = smeshgui.AddSubMeshOnShape( idmesh,
                                    Id_SubFace,
                                    salome.orb.object_to_string(submesh),
                                    ShapeTypeFace )

smeshgui.SetName(idsm2, "SubMeshFace")
smeshgui.SetAlgorithms( idsm2, idqad2 );  # Quadrangle 2D
smeshgui.SetHypothesis( idsm2, idarea2 ); # max area



print "-------------------------- add hypothesis and algorith to sub face"        

sub_face2  = salome.IDToObject( Id_SubFace2 )
submesh    = mesh.GetElementsOnShape( sub_face2 )

ret = mesh.AddHypothesis( sub_face2, algoQad2 )  # Quadrangle 2D
print ret
ret = mesh.AddHypothesis( sub_face2, hypArea2 )  # max area
print ret

idsm3 = smeshgui.AddSubMeshOnShape( idmesh,
                                    Id_SubFace2,
                                    salome.orb.object_to_string(submesh),
                                    ShapeTypeFace )

smeshgui.SetName(idsm3, "SubMeshFace2")
smeshgui.SetAlgorithms( idsm3, idqad2 );  # Quadrangle 2D
smeshgui.SetHypothesis( idsm3, idarea2 ); # max area




sg.updateObjBrowser(1);
