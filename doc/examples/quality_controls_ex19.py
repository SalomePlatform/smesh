# Element Diameter 2D

from mechanic import *

# Criterion : ELEMENT DIAMETER 2D > 10
mel_2d_margin = 10

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_MaxElementLength2D, SMESH.FT_MoreThan, mel_2d_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("Criterion: Element Diameter 2D Ratio > ", mel_2d_margin, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Element Diameter 2D > " + repr(mel_2d_margin))
aGroup.Add(anIds)
