# Borders at Multiconnection

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create open shell: a box without one plane
box = geom_builder.MakeBox(0., 0., 0., 20., 20., 15.)
FaceList = geom_builder.SubShapeAll(box, geom_builder.ShapeType["FACE"])
FaceList.remove(FaceList[5])
box = geom_builder.MakeShell(FaceList)
idbox = geom_builder.addToStudy(box, "box")

# create a mesh
mesh = smesh_builder.Mesh(box, "Mesh_borders_at_multi-connections")
algo = mesh.Segment()
algo.NumberOfSegments(5)
algo = mesh.Triangle()
algo.MaxElementArea(20.)
mesh.Compute() 

# Criterion : Borders at multi-connection
nb_conn = 2

aFilter = smesh_builder.GetFilter(SMESH.EDGE, SMESH.FT_MultiConnection, SMESH.FT_EqualTo, nb_conn)
anIds = mesh.GetIdsFromFilter(aFilter)

# print the result
print("Criterion: Borders at multi-connections Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.GetMesh().CreateGroup(SMESH.EDGE, "Borders at multi-connections")
aGroup.Add(anIds)
