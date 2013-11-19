# Quadrangle Parameters example 2 (using different types)

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
from salome.StdMeshers import StdMeshersBuilder

# Make quadrangle face and explode it on edges.
Vertex_1 = geompy.MakeVertex(0, 0, 0)
Vertex_2 = geompy.MakeVertex(40, 0, 0)
Vertex_3 = geompy.MakeVertex(40, 30, 0)
Vertex_4 = geompy.MakeVertex(0, 30, 0)
Quadrangle_Face_1 = geompy.MakeQuad4Vertices(Vertex_1, Vertex_4, Vertex_3, Vertex_2)
[Edge_1,Edge_2,Edge_3,Edge_4] = geompy.SubShapeAllSorted(Quadrangle_Face_1, geompy.ShapeType["EDGE"])
geompy.addToStudy( Vertex_1, "Vertex_1" )
geompy.addToStudy( Vertex_2, "Vertex_2" )
geompy.addToStudy( Vertex_3, "Vertex_3" )
geompy.addToStudy( Vertex_4, "Vertex_4" )
geompy.addToStudy( Quadrangle_Face_1, "Quadrangle Face_1" )
geompy.addToStudyInFather( Quadrangle_Face_1, Edge_2, "Edge_2" )

# Set the Geometry for meshing
Mesh_1 = smesh.Mesh(Quadrangle_Face_1)

# Create Quadrangle parameters and
# define the Type as Quadrangle Preference
Quadrangle_Parameters_1 = smesh.CreateHypothesis('QuadrangleParams')
Quadrangle_Parameters_1.SetQuadType( StdMeshersBuilder.QUAD_QUADRANGLE_PREF )

# Define other hypotheses and algorithms
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(4)
Nb_Segments_1.SetDistrType( 0 )
status = Mesh_1.AddHypothesis(Quadrangle_Parameters_1)
Quadrangle_2D = Mesh_1.Quadrangle()

# Define submesh on one edge to provide different number of segments
Regular_1D_1 = Mesh_1.Segment(geom=Edge_2)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(10)
Nb_Segments_2.SetDistrType( 0 )
SubMesh_1 = Regular_1D_1.GetSubMesh()

# Compute mesh (with Quadrangle Preference type)
isDone = Mesh_1.Compute()

# Change type to Reduced and compute again
Quadrangle_Parameters_1.SetQuadType( StdMeshersBuilder.QUAD_REDUCED )
isDone = Mesh_1.Compute()
