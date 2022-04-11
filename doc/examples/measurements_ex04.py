# Angle measurement

import salome
salome.salome_init_without_session()

from salome.smesh import smeshBuilder

smesh_builder = smeshBuilder.New()

# use smeshBuilder.GetAngle() to compute angle between 3 arbitrary points

p0 = [1,0,0]
p1 = [0,0,0]
p2 = [0,1,0]

a1 = smesh_builder.GetAngle(p0, p1, p2)
print("Right angle measure", a1 )

# use Mesh.GetAngle() to compute angle between 3 nodes of a mesh

mesh = smesh_builder.Mesh()
n0 = mesh.AddNode( *p0 )
n1 = mesh.AddNode( *p1 )
n2 = mesh.AddNode( *p2 )

a2 = mesh.GetAngle( n0,n1,n2 )
