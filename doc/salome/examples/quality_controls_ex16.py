# Minimum Angle

import SMESH_mechanic

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Criterion : MINIMUM ANGLE < 35.
min_angle = 35.

aFilter = smesh.GetFilter(smesh.FACE, smesh.FT_MinimumAngle, smesh.FT_LessThan, min_angle)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print "Criterion: Minimum Angle < ", min_angle, " Nb = ", len(anIds)
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print ""
  print anIds[i],
  j = j + 1
  pass
print ""

# create a group
aGroup = mesh.CreateEmptyGroup(smesh.FACE, "Minimum Angle < " + `min_angle`)

aGroup.Add(anIds)

salome.sg.updateObjBrowser(1)
