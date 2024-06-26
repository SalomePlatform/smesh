# Taper

from mechanic import *

# Criterion : Taper > 3e-20
taper_margin = 3e-20

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Taper, SMESH.FT_MoreThan, taper_margin)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("Criterion: Taper > ", taper_margin, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Taper > " + repr(taper_margin))
aGroup.Add(anIds)
