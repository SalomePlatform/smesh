# Aspect Ratio

from mechanic import *

# Criterion : ASPECT RATIO > 1.8
ar_margin = 1.8

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_AspectRatio, SMESH.FT_MoreThan, ar_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("Criterion: Aspect Ratio > ", ar_margin, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Aspect Ratio > " + repr(ar_margin))
aGroup.Add(anIds)
