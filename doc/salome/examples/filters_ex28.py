# Belong to Plane

# create mesh
from SMESH_mechanic import *
# create plane
plane_1 = geompy.MakePlane(p3,seg1,2000)
geompy.addToStudy(plane_1, "plane_1")
# get all nodes which lie on the plane \a plane_1
filter = smesh.GetFilter(SMESH.NODE, SMESH.FT_BelongToPlane, plane_1)
ids = mesh.GetIdsFromFilter(filter)
print "Number of nodes which lie on the plane plane_1:", len(ids)
