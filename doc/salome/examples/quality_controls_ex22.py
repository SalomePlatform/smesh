# Element Diameter 3D

import SMESH_mechanic_tetra
import SMESH

smesh  = SMESH_mechanic_tetra.smesh
mesh   = SMESH_mechanic_tetra.mesh
salome = SMESH_mechanic_tetra.salome

# Criterion : ELEMENT DIAMETER 3D > 10
mel_3d_margin = 10

aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_MaxElementLength3D, SMESH.FT_MoreThan, mel_3d_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print "Criterion: Element Diameter 3D Ratio > ", mel_3d_margin, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Element Diameter 3D > " + `mel_3d_margin`)
aGroup.Add(anIds)

salome.sg.updateObjBrowser(True)
