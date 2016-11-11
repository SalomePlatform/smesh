# Linear / quadratic

# create mesh
from SMESH_mechanic import *

# get linear and quadratic edges
filter_linear    = smesh.GetFilter(SMESH.EDGE, SMESH.FT_LinearOrQuadratic)
filter_quadratic = smesh.GetFilter(SMESH.EDGE, SMESH.FT_LinearOrQuadratic, SMESH.FT_LogicalNOT)
ids_linear    = mesh.GetIdsFromFilter(filter_linear)
ids_quadratic = mesh.GetIdsFromFilter(filter_quadratic)
print "Number of linear edges:", len(ids_linear), "; number of quadratic edges:", len(ids_quadratic)

# convert mesh to quadratic
print "Convert to quadratic..."
mesh.ConvertToQuadratic()

# get linear and quadratic edges
ids_linear    = mesh.GetIdsFromFilter(filter_linear)
ids_quadratic = mesh.GetIdsFromFilter(filter_quadratic)
print "Number of linear edges:", len(ids_linear), "; number of quadratic edges:", len(ids_quadratic)
