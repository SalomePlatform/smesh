# Quadrangle Parameters example 2 (using different types)

import salome
salome.salome_init_without_session()
from salome.geom import geomBuilder
geompy = geomBuilder.New()
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

# Make quadrangle face and explode it on edges.
Vertex_1 = geompy.MakeVertex(0, 0, 0)
Vertex_2 = geompy.MakeVertex(40, 0, 0)
Vertex_3 = geompy.MakeVertex(40, 30, 0)
Vertex_4 = geompy.MakeVertex(0, 30, 0)
Quadrangle_Face_1 = geompy.MakeQuad4Vertices(Vertex_1, Vertex_4, Vertex_3, Vertex_2)
[Edge_1,Edge_2,Edge_3,Edge_4] = geompy.SubShapeAllSorted(Quadrangle_Face_1, geompy.ShapeType["EDGE"])
geompy.addToStudy( Quadrangle_Face_1, "Quadrangle Face_1" )
geompy.addToStudyInFather( Quadrangle_Face_1, Edge_2, "Edge_2" )

# Set the Geometry for meshing
Mesh_1 = smesh.Mesh(Quadrangle_Face_1)

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
isDone = Mesh_1.Compute()

# Change type to Reduced and compute again
Quadrangle_Parameters_1.SetQuadType( smeshBuilder.QUAD_REDUCED )
isDone = Mesh_1.Compute()
