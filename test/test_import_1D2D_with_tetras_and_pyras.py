#!/usr/bin/env python

# test fix bos #33557 Bad pyramids generated

import sys
import salome


#nb_segs_right = 30
#nb_segs_right = 100
nb_segs_right = 150
#nb_segs_right = 200

algo = "MG-Tetra"
#algo = "Netgen"
#algo = "GMSH"

algo_gmsh = "Delaunay"
#algo_gmsh = "HXT"

salome.salome_init()



###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS


geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Vertex_1 = geompy.MakeVertex(-0.62375, 0.0575, 0.02)
Vertex_2 = geompy.MakeVertex(-0.62375, -0.0575, 0.02)
Vertex_3 = geompy.MakeVertex(-0.365, -0.0575, 0.02)
Vertex_4 = geompy.MakeVertex(-0.365, 0.0575, 0.02)
geomObj_1 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
Curve_1 = geompy.MakePolyline([Vertex_1, Vertex_4, Vertex_3, Vertex_2], True)
Face_1 = geompy.MakeFaceWires([Curve_1], 1)
[right_side] = geompy.SubShapes(Face_1, [6])
[right_side] = geompy.GetExistingSubObjects(Face_1, False)
path = geompy.MakePrismDXDYDZ(Vertex_3, 0, 0, 0.1)
Translation_1 = geompy.MakeTranslation(Vertex_3, 0, -0.02, 0)
Translation_2 = geompy.MakeTranslation(Vertex_4, 0, 0.02, 0)
Translation_3 = geompy.MakeTranslation(Vertex_4, 0, 0.02, 0)
Line_1 = geompy.MakeLineTwoPnt(Translation_3, Translation_1)
Extrusion_1 = geompy.MakePrismDXDYDZ(Line_1, 0.2, 0, 0)
Partition_1 = geompy.MakePartition([Extrusion_1], [Face_1], [], [], geompy.ShapeType["FACE"], 0, [], 0)
Extrusion_2 = geompy.MakePrismDXDYDZ(Partition_1, 0, 0, 0.1)
[Face_to_enforce] = geompy.SubShapes(Extrusion_2, [30])
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( Curve_1, 'Curve_1' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudyInFather( Face_1, right_side, 'right_side' )
geompy.addToStudy( path, 'path' )
geompy.addToStudy( Translation_2, 'Translation_2' )
geompy.addToStudy( Translation_1, 'Translation_1' )
geompy.addToStudy( Translation_3, 'Translation_3' )
geompy.addToStudy( Line_1, 'Line_1' )
geompy.addToStudy( Extrusion_1, 'Extrusion_1' )
geompy.addToStudy( Partition_1, 'Partition_1' )
geompy.addToStudy( Extrusion_2, 'Extrusion_2' )
geompy.addToStudyInFather( Extrusion_2, Face_to_enforce, 'Face_to_enforce' )


# Create a group with other faces than Face_to_enforce
all_faces = geompy.SubShapeAll(Extrusion_2, geompy.ShapeType["FACE"])
gr_other_faces = geompy.CreateGroup(Extrusion_2, geompy.ShapeType["FACE"])
geompy.UnionList(gr_other_faces, all_faces)
geompy.DifferenceList(gr_other_faces, [Face_to_enforce])
geompy.addToStudyInFather( Extrusion_2, gr_other_faces, 'other_faces' )

geom_volume = geompy.BasicProperties(Extrusion_2)[2]

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

Mesh_1 = smesh.Mesh(Face_1,'Mesh_1')

Regular_1D = Mesh_1.Segment()
Number_of_Segments_1 = Regular_1D.NumberOfSegments(10)

Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)

right_side_1 = Mesh_1.GroupOnGeom(right_side,'right_side',SMESH.EDGE)
Regular_1D_1 = Mesh_1.Segment(geom=right_side)


Number_of_Segments_2 = Regular_1D_1.NumberOfSegments(nb_segs_right)
Propagation_of_1D_Hyp = Regular_1D_1.Propagation()

isDone = Mesh_1.Compute()

Mesh_path = smesh.Mesh(path,'Mesh_path')

Regular_1D_2 = Mesh_path.Segment()
Local_Length_1 = Regular_1D_2.LocalLength(0.01,None,1e-07)

isDone = Mesh_path.Compute()

([ right_side_extruded, right_side_top ], error) = Mesh_1.ExtrusionAlongPathObjects( [], [ Mesh_1 ], [ Mesh_1 ], Mesh_path, None, 1, 0, [  ], 0, 0, [ 0, 0, 0 ], 1, [  ], 0 )

Mesh_2 = smesh.Mesh(Extrusion_2,'Mesh_2')

# Enforce elements from Mesh_1
Import_1D2D = Mesh_2.UseExisting2DElements(geom=Face_to_enforce)
Source_Faces_1 = Import_1D2D.SourceFaces([ right_side_extruded ],0,0)

if algo != "GMSH":
  # CADSurf is global mesh
  MG_CADSurf = Mesh_2.Triangle(algo=smeshBuilder.MG_CADSurf)
else:
  # CADSurf is a submesh on other faces
  MG_CADSurf = Mesh_2.Triangle(algo=smeshBuilder.MG_CADSurf, geom=gr_other_faces)
MG_CADSurf_Parameters_1 = MG_CADSurf.Parameters()
MG_CADSurf_Parameters_1.SetGeometricMesh( 0 )
MG_CADSurf_Parameters_1.SetPhySize( 0.005 )
MG_CADSurf_Parameters_1.SetMinSize( 0.005 )
MG_CADSurf_Parameters_1.SetMaxSize( 0.005 )
MG_CADSurf_Parameters_1.SetCorrectSurfaceIntersection( False )
MG_CADSurf_Parameters_1.SetElementType( 1 ) # Quadrangle preference
if algo == "MG-Tetra":
  Mesh_2.Tetrahedron(algo=smeshBuilder.MG_Tetra)
elif algo == "Netgen":
  Mesh_2.Tetrahedron()
elif algo == "GMSH":
  GMSH = Mesh_2.Tetrahedron(algo=smeshBuilder.GMSH)
  Gmsh_Parameters = GMSH.Parameters()
  Gmsh_Parameters.Set2DAlgo( 0 )
  Gmsh_Parameters.SetMinSize( 0 )
  Gmsh_Parameters.SetMaxSize(  0.005 )
  if algo_gmsh == "HXT":
    Gmsh_Parameters.Set3DAlgo( 4 ) 
  else:
    Gmsh_Parameters.Set3DAlgo( 0 ) 
  Gmsh_Parameters.SetIs2d( 0 )

Face_to_enforce_1 = Mesh_2.GroupOnGeom(Face_to_enforce,'Face_to_enforce',SMESH.FACE)


isDone = Mesh_2.Compute()

Mesh_2.MakeGroup("pyramids", SMESH.VOLUME, CritType=SMESH.FT_ElemGeomType, Threshold=SMESH.Geom_PYRAMID)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()

if not isDone:
  raise Exception("Error on mesh compute")

mesh_volume = Mesh_2.GetVolume()
assert abs(mesh_volume-geom_volume)/geom_volume < 1e-7, "Wrong mesh volume"

min_aspect_ratio, max_aspect_ratio = Mesh_2.GetMinMax(SMESH.FT_AspectRatio3D)
assert max_aspect_ratio < 200, "Bad aspect ratio 3D: %.1f"%max_aspect_ratio

min_volume, max_volume = Mesh_2.GetMinMax(SMESH.FT_Volume3D)
assert min_volume > 1e-10, "Bad min volume: %s"%min_volume
