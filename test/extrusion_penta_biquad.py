# -*- coding: utf-8 -*-

import sys
from salome.kernel import salome

salome.salome_init_without_session()

from salome.kernel import GEOM
from salome.geom import geomBuilder
import math
from salome.kernel import SALOMEDS

geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Divided_Disk_1 = geompy.MakeDividedDisk(100, 1, GEOM.SQUARE)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Divided_Disk_1, 'Divided Disk_1' )

from salome.kernel import SMESH
from salome.kernel import  SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
aFilterManager = smesh.CreateFilterManager()
Mesh_1 = smesh.Mesh(Divided_Disk_1)
Regular_1D = Mesh_1.Segment()
Number_of_Segments_1 = Regular_1D.NumberOfSegments(6)
Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
isDone = Mesh_1.Compute()
if not isDone:
    raise Exception("Error when computing Mesh")

aMaxElementLength2D0x5d7fdf0 = aFilterManager.CreateMaxElementLength2D()
isDone = Mesh_1.QuadToTriObject( Mesh_1,  )
Mesh_1.ExtrusionSweepObjects( [ Mesh_1 ], [ Mesh_1 ], [ Mesh_1 ], [ 0, 0, 50 ], 3, 1 )
Mesh_1.ConvertToQuadratic(0, Mesh_1,True)

infos = Mesh_1.GetMeshInfo()
print("Number of biquadratic pentahedrons:", infos[SMESH.Entity_BiQuad_Penta])
if (infos[SMESH.Entity_BiQuad_Penta] != 1080):
  raise RuntimeError("Bad number of biquadratic pentahedrons: should be 1080")
  
## Set names of Mesh objects
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Number_of_Segments_1, 'Number of Segments_1')
smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
