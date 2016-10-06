# Borders at Multiconnection 2D


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a compound of two glued boxes
box1 = geompy.MakeBox(0., 0., 0., 20., 20., 15.)
box2 = geompy.MakeTranslation(box1, 0., 20., 0)
comp = geompy.MakeCompound([box1, box2])
box = geompy.MakeGlueFaces(comp, 0.000001)
idbox = geompy.addToStudy(box, "box")

# create a mesh
mesh = smesh.Mesh(box, "Box compound : 2D triangle mesh")
algo = mesh.Segment()
algo.NumberOfSegments(5)
algo = mesh.Triangle()
algo.MaxElementArea(20.)
mesh.Compute() 

# Criterion : MULTI-CONNECTION 2D = 3
nb_conn = 3

aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_MultiConnection2D, SMESH.FT_EqualTo, nb_conn)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print "Criterion: Borders at multi-connection 2D = ", nb_conn, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Borders at multi-connection 2D = " + `nb_conn`)
aGroup.Add(anIds)

salome.sg.updateObjBrowser(True)
