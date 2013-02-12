# Double edges, Double faces, Double volumes

from smesh import *
# make a mesh on a box
box = geompy.MakeBoxDXDYDZ(100,100,100)
mesh = Mesh( box, "Box" )
mesh.Segment().NumberOfSegments(10)
mesh.Quadrangle()
mesh.Hexahedron()
mesh.Compute()
# copy all elements with translation and Merge nodes
mesh.TranslateObject( mesh, MakeDirStruct( 10,0,0), Copy=True )
mesh.MergeNodes( mesh.FindCoincidentNodes(1e-7) )
# create filters to find equal elements
equalEdgesFilter   = GetFilter(SMESH.EDGE, FT_EqualEdges)
equalFacesFilter   = GetFilter(SMESH.FACE, FT_EqualFaces)
equalVolumesFilter = GetFilter(SMESH.VOLUME, FT_EqualVolumes)
# get equal elements
print "Number of equal edges:",   len( mesh.GetIdsFromFilter( equalEdgesFilter ))
print "Number of equal faces:",   len( mesh.GetIdsFromFilter( equalFacesFilter ))
print "Number of equal volumes:", len( mesh.GetIdsFromFilter( equalVolumesFilter ))
