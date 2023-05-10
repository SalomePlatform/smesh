#!/usr/bin/env python

import sys
import salome

salome.salome_init()

###
### SHAPER component
###

from salome.shaper import model

model.begin()
partSet = model.moduleDocument()

### Create Part
Part_1 = model.addPart(partSet)
Part_1_doc = Part_1.document()

### Create Cylinder
Cylinder_1 = model.addCylinder(Part_1_doc, model.selection("VERTEX", "PartSet/Origin"), model.selection("EDGE", "PartSet/OZ"), 0.5, 5)

### Create Point
Point_2 = model.addPoint(Part_1_doc, -3, 0, 1.5)

### Create Cylinder
Cylinder_2 = model.addCylinder(Part_1_doc, model.selection("VERTEX", "Point_1"), model.selection("EDGE", "PartSet/OX"), 0.3, 3)

### Create Fuse
Fuse_1 = model.addFuse(Part_1_doc, [model.selection("SOLID", "Cylinder_1_1"), model.selection("SOLID", "Cylinder_2_1")], keepSubResults = True)

### Create Group
Group_1 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Cylinder_2_1/Face_3")])
Group_1.setName("Inlet_x")
Group_1.result().setName("Inlet_x")
Group_1.result().setColor(0, 85, 255)

### Create Group
Group_2 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Cylinder_1_1/Face_3")])
Group_2.setName("Inlet_z")
Group_2.result().setName("Inlet_z")
Group_2.result().setColor(170, 0, 0)

### Create Group
Group_3 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Cylinder_1_1/Face_2")])
Group_3.setName("Outlet")
Group_3.result().setName("Outlet")
Group_3.result().setColor(255, 170, 0)

### Create Group
Group_4 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Fuse_1_1/Modified_Face&Cylinder_1_1/Face_1"), model.selection("FACE", "Fuse_1_1/Modified_Face&Cylinder_2_1/Face_1")])
Group_4.setName("Wall")
Group_4.result().setName("Wall")
Group_4.result().setColor(85, 255, 0)

### Create Group
Group_5 = model.addGroup(Part_1_doc, "Vertices", [model.selection("VERTEX", "Fuse_1_1/Generated_Vertex&Cylinder_1_1/Face_1")])
Group_5.setName("corner")
Group_5.result().setName("corner")

model.end()

###
### SHAPERSTUDY component
###

model.publishToShaperStudy()
import SHAPERSTUDY
Fuse_1_1, Inlet_x, Inlet_z, Outlet, Wall, corner, = SHAPERSTUDY.shape(model.featureStringId(Fuse_1))

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

Mesh_1 = smesh.Mesh(Fuse_1_1,'Mesh_1')
Cartesian_3D_1 = Mesh_1.BodyFitted()
CartesianParameters3D = Cartesian_3D_1.SetGrid([ [ '0.05' ], [ 0, 1 ]],[ [ '0.05' ], [ 0, 1 ]],[ [ '0.05' ], [ 0, 1 ]],5e+07,1)
CartesianParameters3D.SetToCreateFaces( 1 )
ViscousLayers_0_05_3 = Cartesian_3D_1.ViscousLayers(0.05,3,1.1,[ Inlet_x, Inlet_z, Outlet ],1,smeshBuilder.SURF_OFFSET_SMOOTH,'Viscous_Layers')

isDone = Mesh_1.Compute()

if not isDone:
  raise Exception("Mesh_1 not computed")

# Check volume of mesh
volume_mesh = Mesh_1.GetVolume()
volume_geom = 4.640535669879

assert abs(volume_mesh-volume_geom)/volume_geom < 0.005

# Check volume of Viscous layer
gr_Viscous_Layers = Mesh_1.GetGroupByName("Viscous_Layers")[0]

volume_mesh_VL = smesh.GetVolume(gr_Viscous_Layers)
volume_geom_reduced = 3.685108989376
volume_geom_VL = volume_geom - volume_geom_reduced

assert abs(volume_mesh_VL-volume_geom_VL)/volume_geom_VL < 0.005



if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
