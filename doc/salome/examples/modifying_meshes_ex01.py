# Add Node


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

# add node
new_id = mesh.AddNode(50, 10, 0)
print ""
if new_id == 0: print "KO node addition."
else:           print "New Node has been added with ID ", new_id
