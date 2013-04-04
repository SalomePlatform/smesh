# Range of IDs

# create mesh
from SMESH_mechanic import *
# get nodes with identifiers [5-10] and [15-30]
criterion1 = smesh.GetCriterion(SMESH.NODE, SMESH.FT_RangeOfIds, Threshold="5-10",\
                                BinaryOp=SMESH.FT_LogicalOR)
criterion2 = smesh.GetCriterion(SMESH.NODE, SMESH.FT_RangeOfIds, Threshold="15-30")
filter = smesh.CreateFilterManager().CreateFilter()
filter.SetCriteria([criterion1,criterion2])
ids = mesh.GetIdsFromFilter(filter)
print "Number of nodes in ranges [5-10] and [15-30]:", len(ids)
