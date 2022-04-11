# Length 2D

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
mesh = smesh_builder.Mesh(box, "Mesh_Length_2D")
algo = mesh.Segment()
algo.NumberOfSegments(5)
algo = mesh.Triangle()
algo.MaxElementArea(20.)
mesh.Compute()

# Criterion : Length 2D > 5.7
length_margin = 5.7

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Length2D, SMESH.FT_MoreThan, length_margin)

anIds = mesh.GetIdsFromFilter(aFilter)

# print the result
print("Criterion: Edges length 2D > ", length_margin, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Faces with length 2D > " + repr(length_margin))
aGroup.Add(anIds)
