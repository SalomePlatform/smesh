# Over-constrained faces

import salome
salome.salome_init_without_session()

import SMESH
from salome.smesh import smeshBuilder

smesh_builder = smeshBuilder.New()

mesh = smesh_builder.Mesh()
faceFilter = smesh_builder.GetFilter(SMESH.FACE,SMESH.FT_OverConstrainedFace)

#make an edge
n1 = mesh.AddNode(0,0,0)
n2 = mesh.AddNode(10,0,0)
edge = mesh.AddEdge([n1,n2])
assert( not mesh.GetIdsFromFilter( faceFilter ))

# make faces
mesh.ExtrusionSweep([edge], smesh_builder.MakeDirStruct(0,7,0), 5)
assert( 5 == len( mesh.GetIdsFromFilter( faceFilter )))
