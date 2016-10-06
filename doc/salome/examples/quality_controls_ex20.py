# Aspect Ratio 3D

import SMESH_mechanic_tetra 
import SMESH

smesh  = SMESH_mechanic_tetra.smesh
mesh   = SMESH_mechanic_tetra.mesh
salome = SMESH_mechanic_tetra.salome

# Criterion : ASPECT RATIO 3D > 4.5
ar_margin = 4.5

aFilter = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_AspectRatio3D, SMESH.FT_MoreThan, ar_margin)

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
aGroup = mesh.CreateEmptyGroup(SMESH.VOLUME, "Aspect Ratio 3D > " + `ar_margin`)

aGroup.Add(anIds)

salome.sg.updateObjBrowser(True)
