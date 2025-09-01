# Free Borders

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
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
mesh = smesh_builder.Mesh(box, "Mesh_free_borders")
algo = mesh.Segment()
algo.NumberOfSegments(5)
algo = mesh.Triangle()
algo.MaxElementArea(20.)
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# criterion : free borders
aFilter = smesh_builder.GetFilter(SMESH.EDGE, SMESH.FT_FreeBorders)
anIds = mesh.GetIdsFromFilter(aFilter)

# print the result
print("Criterion: Free borders Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.GetMesh().CreateGroup(SMESH.EDGE, "Free borders")
aGroup.Add(anIds)
