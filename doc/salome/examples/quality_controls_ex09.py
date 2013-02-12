# Over-constrained faces

from smesh import *
SetCurrentStudy(salome.myStudy)

mesh = Mesh()
faceFilter = GetFilter(FACE,FT_OverConstrainedFace)

#make an edge
n1 = mesh.AddNode(0,0,0)
n2 = mesh.AddNode(10,0,0)
edge = mesh.AddEdge([n1,n2])
assert( not mesh.GetIdsFromFilter( faceFilter ))

# make faces 
mesh.ExtrusionSweep([edge], MakeDirStruct(0,7,0), 5)
assert( 2 == len( mesh.GetIdsFromFilter( faceFilter )))
