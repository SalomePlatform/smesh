# Taper

import SMESH_mechanic
import SMESH

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Criterion : Taper > 3e-20
taper_margin = 3e-20

aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Taper, SMESH.FT_MoreThan, taper_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print "Criterion: Taper > ", taper_margin, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Taper > " + `taper_margin`)
aGroup.Add(anIds)

salome.sg.updateObjBrowser(True)
