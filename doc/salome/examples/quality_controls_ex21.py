# Volume

import SMESH_mechanic_tetra

smesh  = SMESH_mechanic_tetra.smesh
mesh   = SMESH_mechanic_tetra.mesh
salome = SMESH_mechanic_tetra.salome

# Criterion : VOLUME < 7.
volume_margin = 7.

aFilter = smesh.GetFilter(smesh.VOLUME, smesh.FT_Volume3D, smesh.FT_LessThan, volume_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print ""
print "Criterion: Volume < ", volume_margin, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.CreateEmptyGroup(smesh.VOLUME, "Volume < " + `volume_margin`)

aGroup.Add(anIds)

salome.sg.updateObjBrowser(1) 
