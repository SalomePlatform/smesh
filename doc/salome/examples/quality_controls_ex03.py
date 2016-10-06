# Length 1D


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create open shell: a box without one plane
box = geompy.MakeBox(0., 0., 0., 20., 20., 15.)
FaceList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
FaceList.remove(FaceList[5])
box = geompy.MakeShell(FaceList)
idbox = geompy.addToStudy(box, "box")

# create a mesh
mesh = smesh.Mesh(box, "Mesh_Length_1D")
algo = mesh.Segment()
algo.NumberOfSegments(5)
algo = mesh.Triangle()
algo.MaxElementArea(20.)
mesh.Compute() 

# Criterion : Length > 3.
length_margin = 3.

aFilter = smesh.GetFilter(SMESH.EDGE, SMESH.FT_Length, SMESH.FT_MoreThan, length_margin)
anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print "Criterion: Edges length > ", length_margin, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.GetMesh().CreateGroup(SMESH.EDGE, "Edges with length > " + `length_margin`)
aGroup.Add(anIds)

salome.sg.updateObjBrowser(True)
