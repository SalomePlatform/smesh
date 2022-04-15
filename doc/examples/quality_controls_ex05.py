# Free Nodes

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create box
box = geom_builder.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geom_builder.addToStudy(box, "box")

# create a mesh
mesh = smesh_builder.Mesh(box, "Mesh_free_nodes")
mesh.Segment().NumberOfSegments(10)
mesh.Triangle().MaxElementArea(150.)
mesh.Compute() 

# Remove some elements to obtain free nodes
# Criterion : AREA < 80.
area_margin = 80.

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_LessThan, area_margin)

anIds = mesh.GetIdsFromFilter(aFilter)

mesh.RemoveElements(anIds)

# criterion : free nodes
aFilter = smesh_builder.GetFilter(SMESH.NODE, SMESH.FT_FreeNodes)
anNodeIds = mesh.GetIdsFromFilter(aFilter)

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.NODE, "Free_nodes")
aGroup.Add(anNodeIds)

# print the result
print("Criterion: Free nodes Nb = ", len(anNodeIds))
j = 1
for i in range(len(anNodeIds)):
  if j > 20: j = 1; print("")
  print(anNodeIds[i], end=' ')
  j = j + 1
  pass
print("")
