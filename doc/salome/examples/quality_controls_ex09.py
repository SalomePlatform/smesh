# Over-constrained faces


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook


mesh = smesh.Mesh()
faceFilter = smesh.GetFilter(SMESH.FACE,SMESH.FT_OverConstrainedFace)

#make an edge
n1 = mesh.AddNode(0,0,0)
n2 = mesh.AddNode(10,0,0)
edge = mesh.AddEdge([n1,n2])
assert( not mesh.GetIdsFromFilter( faceFilter ))

# make faces 
mesh.ExtrusionSweep([edge], smesh.MakeDirStruct(0,7,0), 5)
assert( 2 == len( mesh.GetIdsFromFilter( faceFilter )))
