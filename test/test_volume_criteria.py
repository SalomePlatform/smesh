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
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Box_1, 'Box_1' )

faces = geompy.SubShapeAllSorted(Box_1, geompy.ShapeType["FACE"])
face_1  = faces[0]
geompy.addToStudyInFather( Box_1, face_1, 'face_1' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
#smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                 # multiples meshes built in parallel, complex and numerous mesh edition (performance)

nb_seg = 5

Mesh_1 = smesh.Mesh(Box_1,'Mesh_1')
NETGEN_1D_2D_3D = Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
status = Mesh_1.RemoveHypothesis(NETGEN_1D_2D_3D)
Regular_1D = Mesh_1.Segment()
Number_of_Segments_1 = Regular_1D.NumberOfSegments(nb_seg)
Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
NETGEN_3D = Mesh_1.Hexahedron()
isDone = Mesh_1.Compute()
Mesh_1.CheckCompute()

gr_volume_lying_on_face_1 = Mesh_1.MakeGroup("volume_lying_on_face_1", SMESH.VOLUME, CritType=SMESH.FT_LyingOnGeom, Threshold=face_1)

isDone = Mesh_1.ReorientObject( gr_volume_lying_on_face_1 )

mini, maxi = Mesh_1.GetMinMax(SMESH.FT_Volume3D)
print("mini, maxi:", mini, maxi)

expected_min = -64000
expected_max = 64000

assert abs(mini-expected_min) < 1e-5
assert abs(maxi-expected_max) < 1e-5

gr_negative_volume = Mesh_1.MakeGroup("negative_volumes", SMESH.VOLUME, CritType=SMESH.FT_Volume3D, Compare=SMESH.FT_LessThan, Threshold=0)
assert gr_negative_volume.Size() == nb_seg**2

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
