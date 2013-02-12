# Range of IDs

# create mesh
from SMESH_mechanic import *
# get nodes with identifiers [5-10] and [15-30]
criterion1 = smesh.GetCriterion(smesh.NODE, smesh.FT_RangeOfIds, Treshold="5-10",\
                                BinaryOp=smesh.FT_LogicalOR)
criterion2 = smesh.GetCriterion(smesh.NODE, smesh.FT_RangeOfIds, Treshold="15-30")
filter = smesh.CreateFilterManager().CreateFilter()
filter.SetCriteria([criterion1,criterion2])
ids = mesh.GetIdsFromFilter(filter)
print "Number of nodes in ranges [5-10] and [15-30]:", len(ids)
