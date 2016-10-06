# Uniting two Triangles


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

# create the following mesh:
# .----.----.----.
# |   /|   /|   /|
# |  / |  / |  / |
# | /  | /  | /  |
# |/   |/   |/   |
# .----.----.----.

bb = [0, 0, 0, 0]
tt = [0, 0, 0, 0]
ff = [0, 0, 0, 0, 0, 0]

bb[0] = mesh.AddNode( 0., 0., 0.)
bb[1] = mesh.AddNode(10., 0., 0.)
bb[2] = mesh.AddNode(20., 0., 0.)
bb[3] = mesh.AddNode(30., 0., 0.)

tt[0] = mesh.AddNode( 0., 15., 0.)
tt[1] = mesh.AddNode(10., 15., 0.)
tt[2] = mesh.AddNode(20., 15., 0.)
tt[3] = mesh.AddNode(30., 15., 0.)

ff[0] = mesh.AddFace([bb[0], bb[1], tt[1]])
ff[1] = mesh.AddFace([bb[0], tt[1], tt[0]])
ff[2] = mesh.AddFace([bb[1], bb[2], tt[2]])
ff[3] = mesh.AddFace([bb[1], tt[2], tt[1]])
ff[4] = mesh.AddFace([bb[2], bb[3], tt[3]])
ff[5] = mesh.AddFace([bb[2], tt[3], tt[2]]) 

# delete the diagonal bb[1] - tt[2]
print "\nUnite two triangles ... ",
res = mesh.DeleteDiag(bb[1], tt[2])
if not res: print "failed!"
else:       print "done."

salome.sg.updateObjBrowser(True)
