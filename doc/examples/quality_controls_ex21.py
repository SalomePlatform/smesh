# Volume

from mechanic import *

# Criterion : VOLUME < 7.
volume_margin = 7.

aFilter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_Volume3D, SMESH.FT_LessThan, volume_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("")
print("Criterion: Volume < ", volume_margin, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.VOLUME, "Volume < " + repr(volume_margin))

aGroup.Add(anIds)
