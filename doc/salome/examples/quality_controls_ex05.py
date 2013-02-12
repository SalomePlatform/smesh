# Free Nodes

import salome
import geompy

import smesh

# create box
box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

# create a mesh
mesh = smesh.Mesh(box, "Mesh_free_nodes")
algo = mesh.Segment()
algo.NumberOfSegments(10)
algo = mesh.Triangle(smesh.MEFISTO)
algo.MaxElementArea(150.)
mesh.Compute() 

# Remove some elements to obtain free nodes
# Criterion : AREA < 80.
area_margin = 80.

aFilter = smesh.GetFilter(smesh.FACE, smesh.FT_Area, smesh.FT_LessThan, area_margin)

anIds = mesh.GetIdsFromFilter(aFilter)

mesh.RemoveElements(anIds)

# criterion : free nodes
aFilter = smesh.GetFilter(smesh.NODE, smesh.FT_FreeNodes) 
anNodeIds = mesh.GetIdsFromFilter(aFilter)

# create a group
aGroup = mesh.CreateEmptyGroup(smesh.NODE, "Free_nodes")
aGroup.Add(anNodeIds)

# print the result
print "Criterion: Free nodes Nb = ", len(anNodeIds)
j = 1
for i in range(len(anNodeIds)):
  if j > 20: j = 1; print ""
  print anNodeIds[i],
  j = j + 1
  pass
print ""

salome.sg.updateObjBrowser(1)
