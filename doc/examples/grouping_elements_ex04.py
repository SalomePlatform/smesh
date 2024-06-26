# Edit a Group

from mechanic import *

# Get ids of all faces with area > 35
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 35.)

anIds = mesh.GetIdsFromFilter(aFilter) 

print("Criterion: Area > 35, Nb = ", len(anIds))

# create a group by adding elements with area > 35
aGroup = mesh.CreateEmptyGroup(SMESH.FACE, "Area > 35")
aGroup.Add(anIds) 

# Get ids of all faces with area > 40
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 40.)

anIds = mesh.GetIdsFromFilter(aFilter)

print("Criterion: Area > 40, Nb = ", len(anIds)) 

# create a group of elements with area [35; 40] by removing elements with area > 40 from group aGroup
aGroup.Remove(anIds) 
aGroup.SetName("35 < Area < 40")

# print the result
aGroupElemIDs = aGroup.GetListOfID()

print("Criterion: 35 < Area < 40, Nb = ", len(aGroupElemIDs))

j = 1
for i in range(len(aGroupElemIDs)):
  if j > 20: j = 1; print("")
  print(aGroupElemIDs[i], end=' ')
  j = j + 1
  pass
print("")
