# Free borders

# create mesh
import geompy, smesh, StdMeshers
face = geompy.MakeFaceHW(100, 100, 1)
geompy.addToStudy( face, "quadrangle" )
mesh = smesh.Mesh(face)
mesh.Segment().NumberOfSegments(10)
mesh.Triangle().MaxElementArea(25)
mesh.Compute()
# get all free borders
filter = smesh.GetFilter(smesh.EDGE, smesh.FT_FreeBorders)
ids = mesh.GetIdsFromFilter(filter)
print "Number of edges on free borders:", len(ids)
