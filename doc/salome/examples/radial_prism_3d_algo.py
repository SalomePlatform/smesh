# Usage of Radial Prism 3D meshing algorithm

import salome
salome.salome_init()
from salome.geom import geomBuilder
geompy = geomBuilder.New()
import SMESH
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

# Create geometry: hollow sphere

sphere_1 = geompy.MakeSphereR( 100 )
sphere_2 = geompy.MakeSphereR( 50 )

hollow_sphere = geompy.MakeCut( sphere_1, sphere_2, theName="hollow sphere")

faces = geompy.ExtractShapes( hollow_sphere, geompy.ShapeType["FACE"] )


# Create mesh 

mesh = smesh.Mesh( hollow_sphere, "Mesh of hollow sphere" )

# assign Global Radial Prism algorithm
prism_algo = mesh.Prism()

# define projection between the inner and outer spheres
mesh.Triangle( smeshBuilder.NETGEN_1D2D, faces[0] )    # NETGEN on faces[0]
mesh.Projection1D2D( faces[1] ).SourceFace( faces[0] ) # projection faces[0] -> faces[1]

# define distribution of layers using Number of Segments hypothesis in logarithmic mode
prism_algo.NumberOfSegments( 4, 5. )

# compute the mesh
mesh.Compute()
