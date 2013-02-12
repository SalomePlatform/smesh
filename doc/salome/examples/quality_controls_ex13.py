# Area

import SMESH_mechanic

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Criterion : AREA > 100.
area_margin = 100.

aFilter = smesh.GetFilter(smesh.FACE, smesh.FT_Area, smesh.FT_MoreThan, area_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print "Criterion: Area > ", area_margin, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.CreateEmptyGroup(smesh.FACE, "Area > " + `area_margin`)
aGroup.Add(anIds)

salome.sg.updateObjBrowser(1)  
