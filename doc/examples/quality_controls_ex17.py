# Warping

from mechanic import *

# Criterion : WARP ANGLE > 1e-15
wa_margin = 1e-15

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Warping, SMESH.FT_MoreThan, wa_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("Criterion: Warp > ", wa_margin, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Warp > " + repr(wa_margin))

aGroup.Add(anIds)
