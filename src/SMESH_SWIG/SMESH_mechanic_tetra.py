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

import SMESH
import smeshpy
import salome
from salome import sg
import math

import geompy

# ---------------------------- GEOM --------------------------------------
geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
myBuilder = salome.myStudy.NewBuilder()
#from geompy import gg

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

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

# ---- define a planar face with wire

WantPlanarFace = 1 #True
face1 = geom.MakeFace( wire1, WantPlanarFace )

# ---- create a shape by extrusion

pO = geom.MakePointStruct( 0.0, 0.0,   0.0 )
pz = geom.MakePointStruct( 0.0, 0.0, 100.0 )

prism1    = geom.MakePrism( face1, pO, pz )

# ---- create two cylinders

pc1 = geom.MakePointStruct(  90.0, 50.0, -40.0 )
pc2 = geom.MakePointStruct( 170.0, 70.0, -40.0 )
vz  = geom.MakeDirection( pz )
radius = 20.0
height = 180.0
cyl1  = geom.MakeCylinder( pc1, vz, radius, height )
cyl2  = geom.MakeCylinder( pc2, vz, radius, height )

# ---- cut with cyl1 

shape  = geom.MakeBoolean( prism1, cyl1, 2 )

# ---- fuse with cyl2 to obtain the final mechanic piece :)

mechanic =  geom.MakeBoolean( shape, cyl2, 3 )

idMechanic = geompy.addToStudy( mechanic, "mechanic")

# ---- Analysis of the geometry

print "Analysis of the geometry mechanic :"

subShellList=geompy.SubShapeAll(mechanic,ShapeTypeShell)
subFaceList=geompy.SubShapeAll(mechanic,ShapeTypeFace)
subEdgeList=geompy.SubShapeAll(mechanic,ShapeTypeEdge)

print "number of Shells in mechanic : ",len(subShellList)
print "number of Faces in mechanic : ",len(subFaceList)
print "number of Edges in mechanic : ",len(subEdgeList)

### ---------------------------- SMESH --------------------------------------

# ---- launch SMESH, init a Mesh with shape 'mechanic'

gen  = smeshpy.smeshpy()
mesh = gen.Init( idMechanic )

idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName( idmesh, "Mesh_mechanic" )
smeshgui.SetShape( idMechanic, idmesh )

print "-------------------------- NumberOfSegments"

numberOfSegment = 10

hypNumberOfSegment = gen.CreateHypothesis( "NumberOfSegments" )
hypNbSeg = hypNumberOfSegment._narrow( SMESH.SMESH_NumberOfSegments )
hypNbSeg.SetNumberOfSegments(numberOfSegment)
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()

idSeg = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypNbSeg) )
smeshgui.SetName(idSeg, "NumberOfSegments")

print "-------------------------- MaxElementArea"

maxElementArea = 20

hypMaxElementArea = gen.CreateHypothesis( "MaxElementArea" )
hypArea = hypMaxElementArea._narrow( SMESH.SMESH_MaxElementArea )
hypArea.SetMaxElementArea(maxElementArea)
print hypArea.GetName()
print hypArea.GetId()
print hypArea.GetMaxElementArea()

idArea = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypArea) )
smeshgui.SetName(idArea, "MaxElementArea")

print "-------------------------- MaxElementVolume"

maxElementVolume = 20

hypMaxElementVolume = gen.CreateHypothesis( "MaxElementVolume" )
hypVolume = hypMaxElementVolume._narrow( SMESH.SMESH_MaxElementVolume )
hypVolume.SetMaxElementVolume(maxElementVolume)
print hypVolume.GetName()
print hypVolume.GetId()
print hypVolume.GetMaxElementVolume()

idVolume = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypVolume) )
smeshgui.SetName(idVolume, "MaxElementArea")

print "-------------------------- Regular_1D"

alg1D = gen.CreateHypothesis( "Regular_1D" )
algo1D   = alg1D._narrow( SMESH.SMESH_Algo )
listHyp =algo1D.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
algoReg1D = alg1D._narrow( SMESH.SMESH_Regular_1D )
print algoReg1D.GetName()
print algoReg1D.GetId()

idReg1D = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(algoReg1D) )
smeshgui.SetName( idReg1D, "Regular_1D" )

print "-------------------------- MEFISTO_2D"

alg2D = gen.CreateHypothesis( "MEFISTO_2D" )
algo2D = alg2D._narrow( SMESH.SMESH_Algo )
listHyp = algo2D.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
algoMef = alg2D._narrow( SMESH.SMESH_MEFISTO_2D )
print algoMef.GetName()
print algoMef.GetId()

idMef = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(algoMef) )
smeshgui.SetName( idMef, "MEFISTO_2D" )

print "-------------------------- NETGEN_3D"

alg3D = gen.CreateHypothesis( "NETGEN_3D" )
algo3D = alg3D._narrow( SMESH.SMESH_Algo )
listHyp = algo3D.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
algoNg = alg3D._narrow( SMESH.SMESH_NETGEN_3D )
print algoNg.GetName()
print algoNg.GetId()

idNg = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(algoNg) )
smeshgui.SetName( idNg, "NETGEN_2D" )

print "-------------------------- add hypothesis to main mechanic"

shape_mesh = salome.IDToObject( idMechanic  )
submesh    = mesh.GetElementsOnShape( shape_mesh )

ret = mesh.AddHypothesis( shape_mesh, algoReg1D )   # Regular 1D/wire discretisation
print ret
ret = mesh.AddHypothesis( shape_mesh, algoMef )     # MEFISTO 2D
print ret
ret = mesh.AddHypothesis( shape_mesh, algoNg )     # NETGEN 3D
print ret
ret = mesh.AddHypothesis( shape_mesh, hypNbSeg )   # nb segments
print ret
ret = mesh.AddHypothesis( shape_mesh, hypArea )    # max area
print ret
ret = mesh.AddHypothesis( shape_mesh, hypVolume )    # max volume
print ret

smeshgui.SetAlgorithms( idmesh, idReg1D );  # Regular 1D/wire discretisation
smeshgui.SetAlgorithms( idmesh, idMef );    # MEFISTO 2D
smeshgui.SetAlgorithms( idmesh, idNg );    # NETGEN 3D
smeshgui.SetHypothesis( idmesh, idSeg );    # nb segments
smeshgui.SetHypothesis( idmesh, idArea );  # max area
smeshgui.SetHypothesis( idmesh, idVolume );  # max volume

sg.updateObjBrowser(1);

print "-------------------------- compute the mesh of the mechanic piece"
ret=gen.Compute(mesh,idMechanic)
print ret
log=mesh.GetLog(0) # no erase trace
for linelog in log:
    print linelog

sg.updateObjBrowser(1)
