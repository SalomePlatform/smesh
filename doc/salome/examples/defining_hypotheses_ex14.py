# Quadrangle Parameters example 1 (meshing a face with 3 edges)

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
geompy = geomBuilder.New()

from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

# Get 1/4 part from the disk face.
Box_1 = geompy.MakeBoxDXDYDZ(100, 100, 100)
Disk_1 = geompy.MakeDiskR(100, 1)
Common_1 = geompy.MakeCommon(Disk_1, Box_1)
triaVertex = geompy.GetVertexNearPoint( Common_1, geompy.MakeVertex(0,0,0) )
geompy.addToStudy( Common_1, "Common_1" )
geompy.addToStudyInFather( Common_1, triaVertex, "triaVertex" )


# Set the Geometry for meshing
Mesh_1 = smesh.Mesh(Common_1)

# Define 1D hypothesis
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)

# Create Quadrangle parameters and define the Base Vertex.
Quadrangle_2D = Mesh_1.Quadrangle().TriangleVertex( triaVertex )

# Compute the mesh
Mesh_1.Compute()
