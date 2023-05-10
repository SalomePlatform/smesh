#!/usr/bin/env python

import math
import sys
import salome

# Parameters

r = 6
h = 20

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
Cylinder_1 = model.addCylinder(Part_1_doc, model.selection("VERTEX", "PartSet/Origin"), model.selection("EDGE", "PartSet/OZ"), r, h)

### Create Group
Group_1 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Cylinder_1_1/Face_3")])
Group_1.setName("Inlet")
Group_1.result().setName("Inlet")

### Create Group
Group_2 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Cylinder_1_1/Face_2")])
Group_2.setName("Outlet")
Group_2.result().setName("Outlet")

### Create Group
Group_3 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Cylinder_1_1/Face_1")])
Group_3.setName("Wall")
Group_3.result().setName("Wall")

model.end()

###
### SHAPERSTUDY component
###

model.publishToShaperStudy()
import SHAPERSTUDY
Cylinder_1_1, Inlet, Outlet, Wall, = SHAPERSTUDY.shape(model.featureStringId(Cylinder_1))
###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

Mesh_1 = smesh.Mesh(Cylinder_1_1,'Mesh_1')
Cartesian_3D = Mesh_1.BodyFitted()
Body_Fitting_Parameters_1 = Cartesian_3D.SetGrid([ [ '1' ], [ 0, 1 ]],[ [ '1' ], [ 0, 1 ]],[ [ '1' ], [ 0, 1 ]],10000,1)
Body_Fitting_Parameters_1.SetToCreateFaces( 1 )

width_layer = 1
Viscous_Layers_1 = Cartesian_3D.ViscousLayers(width_layer,3,1.2,[ Inlet, Outlet ],1,smeshBuilder.SURF_OFFSET_SMOOTH,'Viscous_Layers')

isDone = Mesh_1.Compute()

if not isDone:
  raise Exception("Mesh_1 not computed")

# Check volume of mesh
volume_mesh = Mesh_1.GetVolume()
volume_geom = math.pi*r**2*h

assert abs(volume_mesh-volume_geom)/volume_geom < 1e-2

# Check volume of Viscous layer
gr_Viscous_Layers = Mesh_1.GetGroupByName("Viscous_Layers")[0]

volume_mesh_VL = smesh.GetVolume(gr_Viscous_Layers)
volume_geom_VL = volume_geom - math.pi*(r-width_layer)**2*h

assert abs(volume_mesh_VL-volume_geom_VL)/volume_geom_VL < 1e-2


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
