# Belong to Surface

# create mesh
from SMESH_mechanic import *
# create b-spline
spline_1 = geompy.MakeInterpol([p4,p6,p3,p1])
surface_1 = geompy.MakePrismVecH( spline_1, vz, 70.0 )
geompy.addToStudy(surface_1, "surface_1")
# get all nodes which lie on the surface \a surface_1
filter = smesh.GetFilter(SMESH.NODE, SMESH.FT_BelongToGenSurface, surface_1)
ids = mesh.GetIdsFromFilter(filter)
print "Number of nodes which lie on the surface surface_1:", len(ids)
