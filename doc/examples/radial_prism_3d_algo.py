# Usage of Radial Prism 3D meshing algorithm

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Create geometry: hollow sphere

sphere_1 = geom_builder.MakeSphereR( 100 )
sphere_2 = geom_builder.MakeSphereR( 50 )

hollow_sphere = geom_builder.MakeCut( sphere_1, sphere_2, theName="hollow sphere")

faces = geom_builder.ExtractShapes( hollow_sphere, geom_builder.ShapeType["FACE"] )

# Create mesh 

mesh = smesh_builder.Mesh( hollow_sphere, "Mesh of hollow sphere" )

# assign Global Radial Prism algorithm
prism_algo = mesh.Prism()

# define projection between the inner and outer spheres
mesh.Triangle( smeshBuilder.NETGEN_1D2D, faces[0] )    # NETGEN on faces[0]
mesh.Projection1D2D( faces[1] ).SourceFace( faces[0] ) # projection faces[0] -> faces[1]

# define distribution of layers using Number of Segments hypothesis in logarithmic mode
prism_algo.NumberOfSegments( 4, 5. )

# compute the mesh
mesh.Compute()
