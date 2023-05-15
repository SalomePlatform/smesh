#!/usr/bin/env python

import os
import sys
import salome
import tempfile

salome.salome_init()


def getTmpFileName(ext):
  """
  get a tmp file name
  """
  tempdir = tempfile.gettempdir()
  tmp_file = tempfile.NamedTemporaryFile(suffix=".%s"%ext , dir=tempdir, delete=False)
  tmp_filename = tmp_file.name
  return tmp_filename

###
### SHAPER component
###

from salome.shaper import model

model.begin()
partSet = model.moduleDocument()
Part_1 = model.addPart(partSet)
Part_1_doc = Part_1.document()
Cylinder_1 = model.addCylinder(Part_1_doc, model.selection("VERTEX", "PartSet/Origin"), model.selection("EDGE", "PartSet/OZ"), 1, 7)
Point_2 = model.addPoint(Part_1_doc, 1, 0, 1.5)
Point_3 = model.addPoint(Part_1_doc, 0.5, 0, 3.5)
Sphere_1 = model.addSphere(Part_1_doc, model.selection("VERTEX", "Point_1"), 0.75)
Cylinder_2 = model.addCylinder(Part_1_doc, model.selection("VERTEX", "Point_2"), model.selection("EDGE", "PartSet/OX"), 0.6, 3)
Fuse_1_objects_1 = [model.selection("SOLID", "Cylinder_2_1"),
                    model.selection("SOLID", "Cylinder_1_1"),
                    model.selection("SOLID", "Sphere_1_1")]
Fuse_1 = model.addFuse(Part_1_doc, Fuse_1_objects_1, keepSubResults = True)
model.end()

expected_volume = 25.881416712512
model.testResultsVolumes(Fuse_1, [expected_volume])

###
### SHAPERSTUDY component
###

model.publishToShaperStudy()
import SHAPERSTUDY
Fuse_1_1, = SHAPERSTUDY.shape(model.featureStringId(Fuse_1))

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
# Surface Mesh
# ============

Mesh_1 = smesh.Mesh(Fuse_1_1, "Mesh_1")
NETGEN_1D_2D = Mesh_1.Triangle(algo=smeshBuilder.NETGEN_1D2D)
NETGEN_2D_Parameters_1 = NETGEN_1D_2D.Parameters()
NETGEN_2D_Parameters_1.SetMaxSize( 0.75 )
NETGEN_2D_Parameters_1.SetMinSize( 0 )
NETGEN_2D_Parameters_1.SetSecondOrder( 0 )
NETGEN_2D_Parameters_1.SetOptimize( 1 )
NETGEN_2D_Parameters_1.SetFineness( 5 )
NETGEN_2D_Parameters_1.SetGrowthRate( 0.1 )
NETGEN_2D_Parameters_1.SetNbSegPerEdge( 2 )
NETGEN_2D_Parameters_1.SetNbSegPerRadius( 4 )

isDone = Mesh_1.Compute()

if not isDone:
  raise Exception("Error when computing the surface mesh")

# MG-Hexa mesh
# ============

MG_Hexa = Mesh_1.Hexahedron(algo=smeshBuilder.MG_Hexa)
isDone = Mesh_1.Compute()

if not isDone:
  raise Exception("Error when computing volumes with MG-Hexa")

volume = smesh.GetVolume(Mesh_1)
#print("volume: ", volume)
assert abs(volume-expected_volume)/expected_volume < 0.03

assert Mesh_1.NbTriangles() == 0

nb_hexa = Mesh_1.NbHexas()
assert Mesh_1.NbVolumes() == nb_hexa

# MG-Hexa mesh with layers
# ========================

MG_Hexa_Parameters = MG_Hexa.Parameters()
MG_Hexa_Parameters.SetNbLayers( 3 )
MG_Hexa_Parameters.SetFirstLayerSize( 0.01 )
MG_Hexa_Parameters.SetGrowth( 1.1 )
MG_Hexa_Parameters.SetFacesWithLayers( [ 10, 3 ] )
MG_Hexa_Parameters.SetImprintedFaces( [ 18, 20, 22 ] )

isDone = Mesh_1.Compute()

if not isDone:
  raise Exception("Error when computing volumes with MG-Hexa and layers")

nb_hexa_with_layers = Mesh_1.NbHexas()
assert Mesh_1.NbVolumes() == nb_hexa_with_layers
assert nb_hexa < nb_hexa_with_layers

volume_with_layers = smesh.GetVolume(Mesh_1)
#print("volume_with_layers: ", volume_with_layers)
assert abs(volume_with_layers-expected_volume)/expected_volume < 0.05

gr_small_volume = Mesh_1.MakeGroup("small_volumes_layers",
                            SMESH.VOLUME,
                            CritType=SMESH.FT_Volume3D,
                            Compare=SMESH.FT_LessThan,
                            Threshold=8e-5,
                            Tolerance=1e-07)

layers_volume = smesh.GetVolume(gr_small_volume)
#print("layers_volume: ", layers_volume)
assert layers_volume < 0.9

# check max Warp3D
max_warp3D = Mesh_1.GetMinMax(SMESH.FT_Warping3D)[1]
print("max_warp3D: ", max_warp3D)
# Check that some elements are warped
assert max_warp3D > 1

gr_warp3D = Mesh_1.MakeGroup("warp3D",
                            SMESH.VOLUME,
                            CritType=SMESH.FT_Warping3D,
                            Compare=SMESH.FT_MoreThan,
                            Threshold=1,
                            Tolerance=1e-07)

# Check the group has some elements
assert gr_warp3D.Size() > 0

# create a mesh with the faces of the hexaedra thanks to medcoupling
umesh_3D = Mesh_1.ExportMEDCoupling()[0]
# create faces
umesh_2D,d0,d1,d2,d3=umesh_3D.buildDescendingConnectivity()

# export the 2D mesh in a tmp file
tmp_filename = getTmpFileName("med")
umesh_2D.write(tmp_filename)

# import it in SMESH
([Mesh_faces], status) = smesh.CreateMeshesFromMED( tmp_filename )

gr_warp2D = Mesh_faces.MakeGroup("warp2D",
                            SMESH.FACE,
                            CritType=SMESH.FT_Warping,
                            Compare=SMESH.FT_MoreThan,
                            Threshold=1,
                            Tolerance=1e-07)

# check max Warp3D
max_warp2D = Mesh_faces.GetMinMax(SMESH.FT_Warping)[1]
print("max_warp2D: ", max_warp2D)
assert max_warp2D > 1

assert abs(max_warp2D-max_warp3D)/max_warp2D < 1e-5

os.remove(tmp_filename)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
