#!/usr/bin/env python

import sys
import salome

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
Face_1 = geompy.MakeFaceHW(10, 10, 1)
Disk_1 = geompy.MakeDiskR(2.5, 1)
Cut_1 = geompy.MakeCutList(Face_1, [Disk_1])
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( Cut_1, 'Cut_1' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
#smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                 # multiples meshes built in parallel, complex and numerous mesh edition (performance)

Mesh_1 = smesh.Mesh(Face_1,'Mesh_1_quadrangle_mapping')
Regular_1D = Mesh_1.Segment()
Number_of_Segments_1 = Regular_1D.NumberOfSegments(10)
Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
isDone = Mesh_1.Compute()
Mesh_1.CheckCompute()
Mesh_1.ExtrusionSweepObjects( [], [ Mesh_1 ], [ Mesh_1 ], [ 0, 0, 1 ], 5, 1, [  ], 0, [  ], [  ], 0 )

def splitAndCheck(mesh):
  mesh_name = mesh.GetName()
  # default split without asking for no over_constrained volumes
  mesh_copy1 = smesh.CopyMesh(mesh, f"{mesh_name}_split_in_5_default")
  mesh_copy1.SplitVolumesIntoTetra( mesh_copy1, 1, False)
  checkNoOverConstrainedVolume(mesh_copy1, print_only=True)
  # split in 5 tetras with no over_constrained volumes
  mesh_copy2 = smesh.CopyMesh(mesh, f"{mesh_name}_split_in_5_no_over_constrained")
  mesh_copy2.SplitVolumesIntoTetra( mesh_copy2, 1, True)
  checkNoOverConstrainedVolume(mesh_copy2)
  # split in 6 tetras with no over_constrained volumes
  mesh_copy3 = smesh.CopyMesh(mesh, f"{mesh_name}_split_in_5_no_over_constrained")
  mesh_copy3.SplitVolumesIntoTetra( mesh_copy3, 1, True)
  checkNoOverConstrainedVolume(mesh_copy3)

def checkNoOverConstrainedVolume(mesh, print_only=False):
  Group_1 = mesh.MakeGroup("over_constrained", SMESH.VOLUME, CritType=SMESH.FT_OverConstrainedVolume)
  nb_over_constrained_volumes = Group_1.Size()
  print(mesh.GetName(), "Nb of over constrained volumes:", nb_over_constrained_volumes)
  if print_only:
    return
  assert Group_1.Size() == 0

splitAndCheck(Mesh_1)

Mesh_2 = smesh.Mesh(Face_1,'Mesh_2_CADSurf_quadrangles')
MG_CADSurf = Mesh_2.Triangle(algo=smeshBuilder.MG_CADSurf)
MG_CADSurf_Parameters_1 = MG_CADSurf.Parameters()
MG_CADSurf_Parameters_1.SetPhySize( 1.41421 )
MG_CADSurf_Parameters_1.SetMinSize( 0.0141421 )
MG_CADSurf_Parameters_1.SetMaxSize( 2.82843 )
MG_CADSurf_Parameters_1.SetElementType( 2 )
MG_CADSurf_Parameters_1.SetChordalError( 0.707107 )
isDone = Mesh_2.Compute()
Mesh_2.CheckCompute()
Mesh_2.ExtrusionSweepObjects( [], [ Mesh_2 ], [ Mesh_2 ], [ 0, 0, 1 ], 5, 1, [  ], 0, [  ], [  ], 0 )
splitAndCheck(Mesh_2)

Mesh_3 = smesh.Mesh(Face_1,'Mesh_3_netgen_quad_dominated')
NETGEN_1D_2D = Mesh_3.Triangle(algo=smeshBuilder.NETGEN_1D2D)
NETGEN_2D_Parameters_1 = NETGEN_1D_2D.Parameters()
NETGEN_2D_Parameters_1.SetMaxSize( 1.41421 )
NETGEN_2D_Parameters_1.SetMinSize( 0.0141421 )
NETGEN_2D_Parameters_1.SetSecondOrder( 0 )
NETGEN_2D_Parameters_1.SetOptimize( 1 )
NETGEN_2D_Parameters_1.SetFineness( 2 )
NETGEN_2D_Parameters_1.SetChordalError( -1 )
NETGEN_2D_Parameters_1.SetChordalErrorEnabled( 0 )
NETGEN_2D_Parameters_1.SetUseSurfaceCurvature( 1 )
NETGEN_2D_Parameters_1.SetFuseEdges( 1 )
NETGEN_2D_Parameters_1.SetQuadAllowed( 1 )
NETGEN_2D_Parameters_1.SetWorstElemMeasure( 32687 )
NETGEN_2D_Parameters_1.SetUseDelauney( 0 )
NETGEN_2D_Parameters_1.SetCheckChartBoundary( 192 )
isDone = Mesh_3.Compute()
Mesh_3.CheckCompute()
Mesh_3.ExtrusionSweepObjects( [], [ Mesh_3 ], [ Mesh_3 ], [ 0, 0, 1 ], 5, 1, [  ], 0, [  ], [  ], 0 )
splitAndCheck(Mesh_3)

Mesh_4 = smesh.Mesh(Cut_1,'Mesh_4_trou_CADSurf_quadrangles')
MG_CADSurf_1 = Mesh_4.Triangle(algo=smeshBuilder.MG_CADSurf)
MG_CADSurf_Parameters_2 = MG_CADSurf_1.Parameters()
MG_CADSurf_Parameters_2.SetPhySize( 1.41421 )
MG_CADSurf_Parameters_2.SetMinSize( 0.0141421 )
MG_CADSurf_Parameters_2.SetMaxSize( 2.82843 )
MG_CADSurf_Parameters_2.SetElementType( 2 )
MG_CADSurf_Parameters_2.SetChordalError( 0.707107 )
isDone = Mesh_4.Compute()
Mesh_4.CheckCompute()
Mesh_4.ExtrusionSweepObjects( [], [ Mesh_4 ], [ Mesh_4 ], [ 0, 0, 1 ], 5, 1, [  ], 0, [  ], [  ], 0 )
splitAndCheck(Mesh_4)

Mesh_5 = smesh.Mesh(Cut_1,'Mesh_5_trou_CADSurf_quad_dominent')
MG_CADSurf_2 = Mesh_5.Triangle(algo=smeshBuilder.MG_CADSurf)
MG_CADSurf_Parameters_3 = MG_CADSurf_2.Parameters()
MG_CADSurf_Parameters_3.SetPhySize( 1.41421 )
MG_CADSurf_Parameters_3.SetMinSize( 0.0141421 )
MG_CADSurf_Parameters_3.SetMaxSize( 2.82843 )
MG_CADSurf_Parameters_3.SetElementType( 1 )
MG_CADSurf_Parameters_3.SetChordalError( 0.707107 )
isDone = Mesh_5.Compute()
Mesh_5.CheckCompute()
Mesh_5.ExtrusionSweepObjects( [], [ Mesh_5 ], [ Mesh_5 ], [ 0, 0, 1 ], 5, 1, [  ], 0, [  ], [  ], 0 )
splitAndCheck(Mesh_5)

Mesh_6 = smesh.Mesh(Cut_1,'Mesh_6_trou_netgen_quad_dominent')
NETGEN_1D_2D_1 = Mesh_6.Triangle(algo=smeshBuilder.NETGEN_1D2D)
isDone = Mesh_6.Compute()
Mesh_6.CheckCompute()
NETGEN_2D_Parameters_2 = NETGEN_1D_2D_1.Parameters()
NETGEN_2D_Parameters_2.SetMaxSize( 0.5 )
NETGEN_2D_Parameters_2.SetMinSize( 0.25 )
NETGEN_2D_Parameters_2.SetSecondOrder( 0 )
NETGEN_2D_Parameters_2.SetOptimize( 1 )
NETGEN_2D_Parameters_2.SetFineness( 2 )
NETGEN_2D_Parameters_2.SetChordalError( -1 )
NETGEN_2D_Parameters_2.SetChordalErrorEnabled( 0 )
NETGEN_2D_Parameters_2.SetUseSurfaceCurvature( 1 )
NETGEN_2D_Parameters_2.SetFuseEdges( 1 )
NETGEN_2D_Parameters_2.SetQuadAllowed( 1 )
NETGEN_2D_Parameters_2.SetWorstElemMeasure( 32687 )
NETGEN_2D_Parameters_2.SetUseDelauney( 0 )
NETGEN_2D_Parameters_2.SetCheckChartBoundary( 192 )
isDone = Mesh_6.Compute()
Mesh_6.CheckCompute()
Mesh_6.ExtrusionSweepObjects( [], [ Mesh_6 ], [ Mesh_6 ], [ 0, 0, 1 ], 5, 1, [  ], 0, [  ], [  ], 0 )
splitAndCheck(Mesh_6)


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
