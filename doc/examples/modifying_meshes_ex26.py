# Convert mesh to/from quadratic

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create sphere of radius 100

Sphere = geom_builder.MakeSphereR( 100 )
geom_builder.addToStudy( Sphere, "Sphere" )

# create simple tetrahedral mesh

Mesh = smesh_builder.Mesh(Sphere)
Mesh.Segment().NumberOfSegments(5)
Mesh.Triangle()
Mesh.Tetrahedron()

# compute mesh

Mesh.Compute()

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
