# Quadrangle Parameters example 2 (using different types)

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Make quadrangle face and explode it on edges.
Vertex_1 = geom_builder.MakeVertex(0, 0, 0)
Vertex_2 = geom_builder.MakeVertex(40, 0, 0)
Vertex_3 = geom_builder.MakeVertex(40, 30, 0)
Vertex_4 = geom_builder.MakeVertex(0, 30, 0)
Quadrangle_Face_1 = geom_builder.MakeQuad4Vertices(Vertex_1, Vertex_4, Vertex_3, Vertex_2)
[Edge_1,Edge_2,Edge_3,Edge_4] = geom_builder.SubShapeAllSorted(Quadrangle_Face_1, geom_builder.ShapeType["EDGE"])
geom_builder.addToStudy( Quadrangle_Face_1, "Quadrangle Face_1" )
geom_builder.addToStudyInFather( Quadrangle_Face_1, Edge_2, "Edge_2" )

# Set the Geometry for meshing
Mesh_1 = smesh_builder.Mesh(Quadrangle_Face_1)

# Create Quadrangle parameters and
# define the Type as Quadrangle Preference
Quad_algo = Mesh_1.Quadrangle()
Quadrangle_Parameters_1 = Quad_algo.QuadrangleParameters( smeshBuilder.QUAD_QUADRANGLE_PREF )

# Define other hypotheses and algorithms
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(4)

# Define submesh on one edge to provide different number of segments
Regular_1D_1 = Mesh_1.Segment(geom=Edge_2)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(10)

# Compute mesh (with Quadrangle Preference type)
if not Mesh_1.Compute(): raise Exception("Error when computing Mesh")

# Change type to Reduced and compute again
Quadrangle_Parameters_1.SetQuadType( smeshBuilder.QUAD_REDUCED )
if not Mesh_1.Compute(): raise Exception("Error when computing Mesh")
