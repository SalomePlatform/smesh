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

geom  = geompy.geom
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

import StdMeshers
import NETGENPlugin

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
Id_prism1 = geompy.addToStudy( prism1, "prism1")

# ---- create two cylinders

pc1 = geompy.MakeVertex(  90.0, 50.0, -40.0 )
pc2 = geompy.MakeVertex( 170.0, 70.0, -40.0 )
radius = 20.0
height = 180.0
cyl1  = geompy.MakeCylinder( pc1, vz, radius, height )
cyl2  = geompy.MakeCylinder( pc2, vz, radius, height )

Id_Cyl1 = geompy.addToStudy( cyl1, "cyl1" )
Id_Cyl2 = geompy.addToStudy( cyl2, "cyl2" )

# ---- cut with cyl1
shape  = geompy.MakeBoolean( prism1, cyl1, 2 )

# ---- fuse with cyl2 to obtain the final mechanic piece :)
mechanic =  geompy.MakeBoolean( shape, cyl2, 3 )
Id_mechanic = geompy.addToStudy( mechanic, "mechanic" )

# ---- Analysis of the geometry

print "Analysis of the geometry mechanic :"

subShellList = geompy.SubShapeAll(mechanic,geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(mechanic,geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(mechanic,geompy.ShapeType["EDGE"])

print "number of Shells in mechanic : ",len(subShellList)
print "number of Faces in mechanic : ",len(subFaceList)
print "number of Edges in mechanic : ",len(subEdgeList)

### ---------------------------- SMESH --------------------------------------

print "-------------------------- NumberOfSegments"

numberOfSegment = 10

hypNbSeg = smesh.CreateHypothesis( "NumberOfSegments", "libStdMeshersEngine.so" )
hypNbSeg.SetNumberOfSegments( numberOfSegment )
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()

smeshgui.SetName(salome.ObjectToID(hypNbSeg), "NumberOfSegments_10")

print "-------------------------- MaxElementArea"

maxElementArea = 20

hypArea = smesh.CreateHypothesis( "MaxElementArea", "libStdMeshersEngine.so" )
hypArea.SetMaxElementArea( maxElementArea )
print hypArea.GetName()
print hypArea.GetId()
print hypArea.GetMaxElementArea()

smeshgui.SetName(salome.ObjectToID(hypArea), "MaxElementArea_20")

print "-------------------------- MaxElementVolume"

maxElementVolume = 20

hypVolume = smesh.CreateHypothesis( "MaxElementVolume", "libStdMeshersEngine.so" )
hypVolume.SetMaxElementVolume( maxElementVolume )
print hypVolume.GetName()
print hypVolume.GetId()
print hypVolume.GetMaxElementVolume()

smeshgui.SetName(salome.ObjectToID(hypVolume), "MaxElementVolume_20")

print "-------------------------- Regular_1D"

algoReg1D = smesh.CreateHypothesis( "Regular_1D", "libStdMeshersEngine.so" )
listHyp =algoReg1D.GetCompatibleHypothesis()
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

print "-------------------------- NETGEN_3D"

algoNg = smesh.CreateHypothesis( "NETGEN_3D", "libNETGENEngine.so" )
listHyp = algoNg.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algoNg.GetName()
print algoNg.GetId()

smeshgui.SetName(salome.ObjectToID(algoNg), "NETGEN_3D")

print "-------------------------- add hypothesis to main mechanic"

shape_mesh = salome.IDToObject( Id_mechanic  )

mesh = smesh.CreateMesh(shape_mesh)

idmesh = salome.ObjectToID(mesh)
smeshgui.SetName( idmesh, "Mesh_mechanic_tetra" )

mesh.AddHypothesis( shape_mesh, hypNbSeg )   # nb segments
mesh.AddHypothesis( shape_mesh, hypArea )    # max area
mesh.AddHypothesis( shape_mesh, hypVolume )  # max volume

mesh.AddHypothesis( shape_mesh, algoReg1D )  # Regular 1D/wire discretisation
mesh.AddHypothesis( shape_mesh, algoMef )    # MEFISTO 2D
mesh.AddHypothesis( shape_mesh, algoNg )     # NETGEN 3D

print "-------------------------- compute the mesh of the mechanic piece"
smesh.Compute(mesh,shape_mesh)

print "Information about the Mesh_mechanic_tetra:"
print "Number of nodes       : ", mesh.NbNodes()
print "Number of edges       : ", mesh.NbEdges()
print "Number of faces       : ", mesh.NbFaces()
print "Number of triangles   : ", mesh.NbTriangles()
print "Number of quadrangles: ", mesh.NbQuadrangles()
print "Number of volumes     : ", mesh.NbVolumes()
print "Number of tetrahedrons: ", mesh.NbTetras()

salome.sg.updateObjBrowser(1)
