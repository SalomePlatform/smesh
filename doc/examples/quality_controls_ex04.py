# Free Edges

from mechanic import *

aFilterMgr = smesh_builder.CreateFilterManager()

# Remove some elements to obtain free edges
# Criterion : AREA > 95.
area_margin = 95.

aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, area_margin)

anIds = mesh.GetIdsFromFilter(aFilter)

mesh.RemoveElements(anIds)

# Criterion : Free Edges
aBorders = mesh.GetFreeBorders() 

# create groups
aGroupF = mesh.CreateEmptyGroup(SMESH.FACE, "Faces with free edges")
aGroupN = mesh.CreateEmptyGroup(SMESH.NODE, "Nodes on free edges")

# fill groups with elements, corresponding to the criterion
print("")
print("Criterion: Free edges Nb = ", len(aBorders))
for i in range(len(aBorders)):
  aBorder = aBorders[i]
  print("Face # ", aBorder.myElemId, " : Edge between nodes (", end=' ')
  print(aBorder.myPnt1, ", ", aBorder.myPnt2, ")")

  aGroupF.Add([aBorder.myElemId])
  aGroupN.Add([aBorder.myPnt1, aBorder.myPnt2])
