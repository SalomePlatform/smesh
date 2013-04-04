# Convert mesh to/from quadratic


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create sphere of radius 100

Sphere = geompy.MakeSphereR( 100 )
geompy.addToStudy( Sphere, "Sphere" )

# create simple trihedral mesh

Mesh = smesh.Mesh(Sphere)
Regular_1D = Mesh.Segment()
Nb_Segments = Regular_1D.NumberOfSegments(5)
MEFISTO_2D = Mesh.Triangle()
Tetrahedron = Mesh.Tetrahedron()

# compute mesh

isDone = Mesh.Compute()

# convert to quadratic
# theForce3d = 1; this results in the medium node lying at the
# middle of the line segments connecting start and end node of a mesh
# element

Mesh.ConvertToQuadratic( theForce3d=1 )

# revert back to the non-quadratic mesh

Mesh.ConvertFromQuadratic()

# convert to quadratic
# theForce3d = 0; this results in the medium node lying at the
# geometrical edge from which the mesh element is built

Mesh.ConvertToQuadratic( theForce3d=0 )

# to convert not the whole mesh but a sub-mesh, provide it as 
# an additional argument to the functions:
# Mesh.ConvertToQuadratic( 0, subMesh )
# Mesh.ConvertFromQuadratic( subMesh )
#
# Note that the mesh becomes non-conformal at conversion of sub-mesh.
