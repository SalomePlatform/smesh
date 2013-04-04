# Quadrangle Parameters example 1 (meshing a face with 3 edges)

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Get 1/4 part from the disk face.
Box_1 = geompy.MakeBoxDXDYDZ(100, 100, 100)
Disk_1 = geompy.MakeDiskR(100, 1)
Common_1 = geompy.MakeCommon(Disk_1, Box_1)
geompy.addToStudy( Disk_1, "Disk_1" )
geompy.addToStudy( Box_1, "Box_1" )
geompy.addToStudy( Common_1, "Common_1" )

# Set the Geometry for meshing
Mesh_1 = smesh.Mesh(Common_1)


# Define 1D hypothesis and compute the mesh
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Nb_Segments_1.SetDistrType( 0 )

# Create Quadrangle parameters and define the Base Vertex.
Quadrangle_2D = Mesh_1.Quadrangle().TriangleVertex( 8 )

Mesh_1.Compute()
