# Belong to Plane

# create mesh
from mechanic import *

# create plane
plane_1 = geom_builder.MakePlane(p3,seg1,2000)
geom_builder.addToStudy(plane_1, "plane_1")

# get all nodes which lie on the plane \a plane_1
filter = smesh_builder.GetFilter(SMESH.NODE, SMESH.FT_BelongToPlane, plane_1)
ids = mesh.GetIdsFromFilter(filter)
print("Number of nodes which lie on the plane plane_1:", len(ids))
