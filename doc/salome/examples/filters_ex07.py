# Area

# create mesh
from SMESH_mechanic import *
# get faces with area > 60 and < 90
criterion1 = smesh.GetCriterion(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 60,\
                                SMESH.FT_Undefined, SMESH.FT_LogicalAND)
criterion2 = smesh.GetCriterion(SMESH.FACE, SMESH.FT_Area, SMESH.FT_LessThan, 90)
filter = smesh.CreateFilterManager().CreateFilter()
filter.SetCriteria([criterion1,criterion2])
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with area in range (60,90):", len(ids)
