# Area

# create mesh
from mechanic import *

# get faces with area > 60 and < 90
criterion1 = smesh_builder.GetCriterion(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 60)
criterion2 = smesh_builder.GetCriterion(SMESH.FACE, SMESH.FT_Area, SMESH.FT_LessThan, 90)
filter = smesh_builder.GetFilterFromCriteria([criterion1,criterion2], SMESH.FT_LogicalAND)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces with area in range (60,90):", len(ids))
