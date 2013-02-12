# Double nodes

from smesh import *
# make a mesh on a box
box = geompy.MakeBoxDXDYDZ(100,100,100)
mesh = Mesh( box, "Box" )
mesh.Segment().NumberOfSegments(10)
mesh.Quadrangle()
mesh.Hexahedron()
mesh.Compute()
# copy all elements with translation
mesh.TranslateObject( mesh, MakeDirStruct( 10,0,0), Copy=True )
# create filters to find nodes equal within tolerance of 1e-5
filter = GetFilter(SMESH.NODE, FT_EqualNodes, Tolerance=1e-5)
# get equal nodes
print "Number of equal nodes:", len( mesh.GetIdsFromFilter( filter ))
