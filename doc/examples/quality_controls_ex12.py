# Borders at Multiconnection 2D

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a compound of two glued boxes
box1 = geom_builder.MakeBox(0., 0., 0., 20., 20., 15.)
box2 = geom_builder.MakeTranslation(box1, 0., 20., 0)
comp = geom_builder.MakeCompound([box1, box2])
box = geom_builder.MakeGlueFaces(comp, 0.000001)
idbox = geom_builder.addToStudy(box, "box")

# create a mesh
mesh = smesh_builder.Mesh(box, "Box compound : 2D triangle mesh")
algo = mesh.Segment()
algo.NumberOfSegments(5)
algo = mesh.Triangle()
algo.MaxElementArea(20.)
mesh.Compute() 

# Criterion : MULTI-CONNECTION 2D = 3
nb_conn = 3

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_MultiConnection2D, SMESH.FT_EqualTo, nb_conn)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("Criterion: Borders at multi-connection 2D = ", nb_conn, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Borders at multi-connection 2D = " + repr(nb_conn))
aGroup.Add(anIds)
