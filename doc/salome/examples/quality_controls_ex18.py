# Skew

import SMESH_mechanic

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Criterion : Skew > 38.
skew_margin = 38.

aFilter = smesh.GetFilter(smesh.FACE, smesh.FT_Skew, smesh.FT_MoreThan, skew_margin)

anIds = mesh.GetIdsFromFilter(aFilter)

# print the result
print "Criterion: Skew > ", skew_margin, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.CreateEmptyGroup(smesh.FACE, "Skew > " + `skew_margin`)
aGroup.Add(anIds)

salome.sg.updateObjBrowser(1)
