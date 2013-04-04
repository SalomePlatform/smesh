# Minimum Distance

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook

from SMESH_mechanic import mesh as mesh1
from SMESH_test1 import mesh as mesh2

mesh1.Compute()
mesh2.Compute()

# compute min distance from mesh1 to the origin (not available yet)
smesh.MinDistance(mesh1)

# compute min distance from node 10 of mesh1 to the origin
smesh.MinDistance(mesh1, id1=10)
# ... or
mesh1.MinDistance(10)

# compute min distance between nodes 10 and 20 of mesh1
smesh.MinDistance(mesh1, id1=10, id2=20)
# ... or
mesh1.MinDistance(10, 20)

# compute min distance from element 100 of mesh1 to the origin (not available yet)
smesh.MinDistance(mesh1, id1=100, isElem1=True)
# ... or
mesh1.MinDistance(100, isElem1=True)

# compute min distance between elements 100 and 200 of mesh1 (not available yet)
smesh.MinDistance(mesh1, id1=100, id2=200, isElem1=True, isElem2=True)
# ... or
mesh1.MinDistance(100, 200, True, True)

# compute min distance from element 100 to node 20 of mesh1 (not available yet)
smesh.MinDistance(mesh1, id1=100, id2=20, isElem1=True)
# ... or
mesh1.MinDistance(100, 20, True)

# compute min distance from mesh1 to mesh2 (not available yet)
smesh.MinDistance(mesh1, mesh2)

# compute min distance from node 10 of mesh1 to node 20 of mesh2
smesh.MinDistance(mesh1, mesh2, 10, 20)

# compute min distance from node 10 of mesh1 to element 200 of mesh2 (not available yet)
smesh.MinDistance(mesh1, mesh2, 10, 200, isElem2=True)

# etc...

