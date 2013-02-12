# Aspect Ratio 3D

import SMESH_mechanic_tetra 

smesh  = SMESH_mechanic_tetra.smesh
mesh   = SMESH_mechanic_tetra.mesh
salome = SMESH_mechanic_tetra.salome

# Criterion : ASPECT RATIO 3D > 4.5
ar_margin = 4.5

aFilter = smesh.GetFilter(smesh.VOLUME, smesh.FT_AspectRatio3D, smesh.FT_MoreThan, ar_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print "Criterion: Aspect Ratio 3D > ", ar_margin, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.CreateEmptyGroup(smesh.VOLUME, "Aspect Ratio 3D > " + `ar_margin`)

aGroup.Add(anIds)

salome.sg.updateObjBrowser(1)
