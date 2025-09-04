# 1D Mesh with Fixed Points example

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Create face and explode it on edges
face = geom_builder.MakeFaceHW(100, 100, 1)
edges = geom_builder.SubShapeAllSorted(face, geom_builder.ShapeType["EDGE"])
geom_builder.addToStudy( face, "Face" )

# get the first edge from exploded result
edge1 = geom_builder.GetSubShapeID(face, edges[0])

# Define Mesh on previously created face
Mesh_1 = smesh_builder.Mesh(face)

# Create Fixed Point 1D hypothesis and define parameters.
# Note: values greater than 1.0 and less than 0.0 are not taken into account;
# duplicated values are removed. Also, if not specified explicitly, values 0.0 and 1.0
# add added automatically.
# The number of segments should correspond to the number of points (NbSeg = NbPnt-1);
# extra values of segments splitting parameter are not taken into account,
# while missing values are considered to be equal to 1.
Fixed_points_1D_1 = smesh_builder.CreateHypothesis('FixedPoints1D')
Fixed_points_1D_1.SetPoints( [ 1.1, 0.9, 0.5, 0.0, 0.5, -0.3 ] )
Fixed_points_1D_1.SetNbSegments( [ 3, 1, 2 ] )
Fixed_points_1D_1.SetReversedEdges( [edge1] )

# Add hypothesis to mesh and define 2D parameters
Mesh_1.AddHypothesis(Fixed_points_1D_1)
Regular_1D = Mesh_1.Segment()
Quadrangle_2D = Mesh_1.Quadrangle()
# Compute mesh
if not Mesh_1.Compute(): raise Exception("Error when computing Mesh")
