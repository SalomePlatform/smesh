# Quadrangle Parameters example 1 (meshing a face with 3 edges)

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Get 1/4 part from the disk face.
Box_1 = geom_builder.MakeBoxDXDYDZ(100, 100, 100)
Disk_1 = geom_builder.MakeDiskR(100, 1)
Common_1 = geom_builder.MakeCommon(Disk_1, Box_1)
triaVertex = geom_builder.GetVertexNearPoint( Common_1, geom_builder.MakeVertex(0,0,0) )
geom_builder.addToStudy( Common_1, "Common_1" )
geom_builder.addToStudyInFather( Common_1, triaVertex, "triaVertex" )

# Set the Geometry for meshing
Mesh_1 = smesh_builder.Mesh(Common_1)

# Define 1D hypothesis
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)

# Create Quadrangle parameters and define the Base Vertex.
Quadrangle_2D = Mesh_1.Quadrangle().TriangleVertex( triaVertex )

# Compute the mesh
if not Mesh_1.Compute(): raise Exception("Error when computing Mesh")
