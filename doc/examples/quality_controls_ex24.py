# Scaled Jacobian

from mechanic import *

# Criterion : Scaled Jacobian > 0.75
scaledJacobian = 0.75

aFilter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_ScaledJacobian, SMESH.FT_MoreThan, scaledJacobian)

anIds = mesh.GetIdsFromFilter(aFilter) 

# print the result
print("Criterion: Scaled Jacobian > ", scaledJacobian, " Nb = ", len(anIds))
j = 1
for i in range(len(anIds)):
  if j > 20: j = 1; print("")
  print(anIds[i], end=' ')
  j = j + 1
  pass
print("")

# create a group
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Scaled Jacobian > " + repr(scaledJacobian))
aGroup.Add(anIds)