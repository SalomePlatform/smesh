# Aspect Ratio 3D

from mechanic import *

# Criterion : ASPECT RATIO 3D > 4.5
ar_margin = 4.5

aFilter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_AspectRatio3D, SMESH.FT_MoreThan, ar_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("Criterion: Aspect Ratio 3D > ", ar_margin, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.VOLUME, "Aspect Ratio 3D > " + repr(ar_margin))

aGroup.Add(anIds)
