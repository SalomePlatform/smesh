# Free edges

# create mesh
import geompy, smesh, StdMeshers
face = geompy.MakeFaceHW(100, 100, 1)
geompy.addToStudy( face, "quadrangle" )
mesh = smesh.Mesh(face)
mesh.Segment().NumberOfSegments(10)
mesh.Triangle().MaxElementArea(25)
mesh.Compute()
# get all faces with free edges
filter = smesh.GetFilter(smesh.FACE, smesh.FT_FreeEdges)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with free edges:", len(ids)
