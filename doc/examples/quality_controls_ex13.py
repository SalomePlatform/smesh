# Area

from mechanic import *

# Criterion : AREA > 100.
area_margin = 100.

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, area_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("Criterion: Area > ", area_margin, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Area > " + repr(area_margin))
aGroup.Add(anIds)
