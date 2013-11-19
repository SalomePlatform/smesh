# 1D Mesh with Fixed Points example

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
from salome.StdMeshers import StdMeshersBuilder

# Create face and explode it on edges
face = geompy.MakeFaceHW(100, 100, 1)
edges = geompy.SubShapeAllSorted(face, geompy.ShapeType["EDGE"])
geompy.addToStudy( face, "Face" )

# get the first edge from exploded result
edge1 = geompy.GetSubShapeID(face, edges[0])

# Define Mesh on previously created face
Mesh_1 = smesh.Mesh(face)

# Create Fixed Point 1D hypothesis and define parameters.
# Note: values greater than 1.0 and less than 0.0 are not taken into account;
# duplicated values are removed. Also, if not specified explicitly, values 0.0 and 1.0
# add added automatically.
# The number of segments should correspond to the number of points (NbSeg = NbPnt-1);
# extra values of segments splitting parameter are not taken into account,
# while missing values are considered to be equal to 1.
Fixed_points_1D_1 = smesh.CreateHypothesis('FixedPoints1D')
Fixed_points_1D_1.SetPoints( [ 1.1, 0.9, 0.5, 0.0, 0.5, -0.3 ] )
Fixed_points_1D_1.SetNbSegments( [ 3, 1, 2 ] )
Fixed_points_1D_1.SetReversedEdges( [edge1] )

# Add hypothesis to mesh and define 2D parameters
Mesh_1.AddHypothesis(Fixed_points_1D_1)
Regular_1D = Mesh_1.Segment()
Quadrangle_2D = Mesh_1.Quadrangle()
# Compute mesh
Mesh_1.Compute()
