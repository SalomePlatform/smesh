# Warping angle

# create mesh
from SMESH_mechanic import *
# get faces with warping angle = 2.0e-13 with tolerance 5.0e-14
criterion = smesh.GetCriterion(SMESH.FACE, SMESH.FT_Warping, SMESH.FT_EqualTo, 2.0e-13)
criterion.Tolerance = 5.0e-14
filter = smesh.CreateFilterManager().CreateFilter()
filter.SetCriteria([criterion])
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with warping angle = 2.0e-13 (tolerance 5.0e-14):", len(ids)
