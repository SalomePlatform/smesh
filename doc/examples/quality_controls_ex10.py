# Over-constrained volumes

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
from salome.smesh import smeshBuilder

smesh_builder = smeshBuilder.New()

mesh = smesh_builder.Mesh()
volumeFilter = smesh_builder.GetFilter(SMESH.VOLUME,SMESH.FT_OverConstrainedVolume)

# make volumes by extrusion of one face
n1 = mesh.AddNode(0,0,0)
n2 = mesh.AddNode(10,0,0)
edge = mesh.AddEdge([n1,n2])
mesh.ExtrusionSweep([edge], smesh_builder.MakeDirStruct(0,7,0), 1)
mesh.ExtrusionSweep( mesh.GetElementsByType(SMESH.FACE), smesh_builder.MakeDirStruct(0,0,5), 7)
assert( 7 == len( mesh.GetIdsFromFilter( volumeFilter )))
