# Orientation


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook


# create an empty mesh structure
mesh = smesh.Mesh() 

# build five quadrangles:
dx = 10
dy = 20

n1  = mesh.AddNode(0.0 * dx, 0, 0)
n2  = mesh.AddNode(1.0 * dx, 0, 0)
n3  = mesh.AddNode(2.0 * dx, 0, 0)
n4  = mesh.AddNode(3.0 * dx, 0, 0)
n5  = mesh.AddNode(4.0 * dx, 0, 0)
n6  = mesh.AddNode(5.0 * dx, 0, 0)
n7  = mesh.AddNode(0.0 * dx, dy, 0)
n8  = mesh.AddNode(1.0 * dx, dy, 0)
n9  = mesh.AddNode(2.0 * dx, dy, 0)
n10 = mesh.AddNode(3.0 * dx, dy, 0)
n11 = mesh.AddNode(4.0 * dx, dy, 0)
n12 = mesh.AddNode(5.0 * dx, dy, 0)

f1 = mesh.AddFace([n1, n2, n8 , n7 ])
f2 = mesh.AddFace([n2, n3, n9 , n8 ])
f3 = mesh.AddFace([n3, n4, n10, n9 ])
f4 = mesh.AddFace([n4, n5, n11, n10])
f5 = mesh.AddFace([n5, n6, n12, n11]) 

# Change the orientation of the second and the fourth faces.
mesh.Reorient([2, 4])

salome.sg.updateObjBrowser(True)
