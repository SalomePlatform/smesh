# Over-constrained volumes


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
volumeFilter = smesh.GetFilter(SMESH.VOLUME,SMESH.FT_OverConstrainedVolume)

# make volumes by extrusion of one face
n1 = mesh.AddNode(0,0,0)
n2 = mesh.AddNode(10,0,0)
edge = mesh.AddEdge([n1,n2])
mesh.ExtrusionSweep([edge], smesh.MakeDirStruct(0,7,0), 1)
mesh.ExtrusionSweep( mesh.GetElementsByType(SMESH.FACE), smesh.MakeDirStruct(0,0,5), 7)
assert( 2 == len( mesh.GetIdsFromFilter( volumeFilter )))
