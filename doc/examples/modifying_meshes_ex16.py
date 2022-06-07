# Diagonal Inversion

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

smesh_builder = smeshBuilder.New()

# create an empty mesh structure
mesh = smesh_builder.Mesh() 

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

# inverse the diagonal bb[1] - tt[2]
print("\nDiagonal inversion ... ", end=' ')
res = mesh.InverseDiag(bb[1], tt[2])
if not res: print("failed!")
else:       print("done.")
