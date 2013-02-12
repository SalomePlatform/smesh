# Linear / quadratic

# create mesh
from SMESH_mechanic import *
# get number of linear and quadratic edges
filter_linear = smesh.GetFilter(smesh.EDGE, smesh.FT_LinearOrQuadratic)
filter_quadratic = smesh.GetFilter(smesh.EDGE, smesh.FT_LinearOrQuadratic, smesh.FT_LogicalNOT)
ids_linear = mesh.GetIdsFromFilter(filter_linear)
ids_quadratic = mesh.GetIdsFromFilter(filter_quadratic)
print "Number of linear edges:", len(ids_linear), "; number of quadratic edges:", len(ids_quadratic)
# convert mesh to quadratic
print "Convert to quadratic..."
mesh.ConvertToQuadratic(True)
# get number of linear and quadratic edges
ids_linear = mesh.GetIdsFromFilter(filter_linear)
ids_quadratic = mesh.GetIdsFromFilter(filter_quadratic)
print "Number of linear edges:", len(ids_linear), "; number of quadratic edges:", len(ids_quadratic)
