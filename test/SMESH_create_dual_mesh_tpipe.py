#!/usr/bin/env python3

import sys
import salome
import math

salome.salome_init()

import GEOM
from salome.geom import geomBuilder

geompy = geomBuilder.New()

# first cylinder
r1 = 0.5
h1 = 5

# second cylinder
r2 = 0.3
h2 = 3

length_piquage = 1.5

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )

Cylinder_1 = geompy.MakeCylinderRH(r1, h1)
Cylinder_2 = geompy.MakeCylinderRH(r2, h2)
Rotation_1 = geompy.MakeRotation(Cylinder_2, OY, -90*math.pi/180.0)
Translation_1 = geompy.MakeTranslation(Rotation_1, 0, 0, length_piquage)

tpipe = geompy.MakeFuseList([Cylinder_1, Translation_1], True, True)
geompy.addToStudy( tpipe, 'tpipe' )

Inlet_z = geompy.GetFaceNearPoint(tpipe, O)
geompy.addToStudyInFather( tpipe, Inlet_z, 'Inlet_z' )

p_inlet_x = geompy.MakeVertex(-h2, 0, length_piquage)
Inlet_x = geompy.GetFaceNearPoint(tpipe, p_inlet_x)
geompy.addToStudyInFather( tpipe, Inlet_x, 'Inlet_x' )

p_outlet = geompy.MakeVertex(0, 0, h1)
Outlet = geompy.GetFaceNearPoint(tpipe, p_outlet)
geompy.addToStudyInFather( tpipe, Outlet, 'Outlet' )

Wall = geompy.CreateGroup(tpipe, geompy.ShapeType["FACE"])
faces = geompy.SubShapeAll(tpipe, geompy.ShapeType["FACE"])
geompy.UnionList(Wall, faces)
geompy.DifferenceList(Wall, [Inlet_x, Inlet_z, Outlet])
geompy.addToStudyInFather( tpipe, Wall, 'Wall' )

p_corner = geompy.MakeVertex(-r2, 0, length_piquage+r2)
corner = geompy.GetVertexNearPoint(tpipe, p_corner)
geompy.addToStudyInFather( tpipe, corner, 'corner' )

geom_groups = [Inlet_x, Inlet_z, Outlet, Wall]

volumeGEOM = geompy.BasicProperties(tpipe)[2]

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

from salome.StdMeshers import StdMeshersBuilder

smesh = smeshBuilder.New()

# Coarse mesh with default hypothesis
Mesh_1 = smesh.Mesh(tpipe, "Mesh_coarse")

Mesh_1.Triangle(algo=smeshBuilder.NETGEN_1D2D)

Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_3D)
isDone = Mesh_1.Compute()

# Create groups
d_mesh_groups = {}
for geom_group in geom_groups:
  gr = Mesh_1.Group(geom_group)
  gr_name = gr.GetName()
  d_mesh_groups[gr_name] = gr

# Check tetra mesh volume
mesh_volume = smesh.GetVolume(Mesh_1)
shape_volume = geompy.BasicProperties(tpipe)[2]
assert abs(mesh_volume-shape_volume)/shape_volume < 0.05

dual_Mesh_raw_1 = smesh.CreateDualMesh(Mesh_1, 'dual_Mesh_raw_1', False)

# check polyhedrons
assert dual_Mesh_raw_1.NbPolyhedrons() > 0
assert dual_Mesh_raw_1.NbPolyhedrons() == dual_Mesh_raw_1.NbVolumes()

# Check dual mesh volume
dual_raw_volume = dual_Mesh_raw_1.GetVolume()
assert abs(dual_raw_volume-shape_volume)/shape_volume < 0.11

# Check groups
dual_Mesh_raw_groups = dual_Mesh_raw_1.GetGroups()
dual_Mesh_raw_group_names = dual_Mesh_raw_1.GetGroupNames()

assert len(dual_Mesh_raw_groups) == 4

for gr_dual, gr_name in zip(dual_Mesh_raw_groups, dual_Mesh_raw_group_names):
  gr_name = gr_name.strip()
  gr_tri = d_mesh_groups[gr_name]
  area_gr_tri = smesh.GetArea(gr_tri)
  area_gr_dual = smesh.GetArea(gr_dual)
  print(gr_name)
  print("Area tri: ", area_gr_tri)
  print("Area dual:", area_gr_dual)
  assert abs(area_gr_tri-area_gr_dual)/area_gr_tri < 1e-3

#dual_Mesh_1 = smesh.CreateDualMesh(Mesh_1, 'dual_Mesh_1', True)


#dual_volume = dual_Mesh_1.GetVolume()
#dual_raw_volume = dual_Mesh_raw_1.GetVolume()
#print("dual_volume: ", dual_volume)
#print("dual_raw_volume: ", dual_raw_volume)

#assert (dual_volume >= dual_raw_volume)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
