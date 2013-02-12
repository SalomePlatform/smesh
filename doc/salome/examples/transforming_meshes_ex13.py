# Reorient faces by vector

import smesh, geompy, SMESH

# create a geometry consisting of two faces
box = geompy.MakeBoxDXDYDZ( 10, 10, 10 )
faces = geompy.SubShapeAllSorted( box, geompy.ShapeType["FACE"])

shape = geompy.MakeCompound( faces[:2] )
faces = geompy.SubShapeAll( shape, geompy.ShapeType["FACE"] )
geompy.addToStudy( shape, "shape")
geompy.addToStudyInFather( shape, faces[0], "faces[0]")
geompy.addToStudyInFather( shape, faces[1], "faces[1]")

# create a 2D mesh
mesh = smesh.Mesh( shape, "test_Reorient2D")
mesh.AutomaticHexahedralization(0.5)
localAlgo = mesh.Segment(faces[0])
localAlgo.NumberOfSegments( 11 )
mesh.Compute()
group = mesh.Group( faces[1] )

vec = geompy.MakeVectorDXDYDZ( 1, 1, 1 )

# Each of arguments of Reorient2D() function can be of different types:
#
# 2DObject    - the whole mesh
# Direction   - a GEOM object (vector)
# FaceOrPoint - an ID of face
mesh.Reorient2D( mesh, vec, mesh.NbElements() )
#
# 2DObject    - a sub-mesh
# Direction   - components of a vector
# FaceOrPoint - a GEOM object (vertex)
mesh.Reorient2D( localAlgo.GetSubMesh(), [ 1, -1, 1 ], geompy.GetFirstVertex( vec ))
#
# 2DObject    - a group of faces
# Direction   - a SMESH.DirStruct structure
# FaceOrPoint - coordinates of a point
mesh.Reorient2D( group, smesh.MakeDirStruct( -10, 1, 10 ), [0,0,0])
#
# FaceOrPoint - a SMESH.PointStruct structure
mesh.Reorient2D( localAlgo.GetSubMesh().GetIDs(), [10,1,0], SMESH.PointStruct(0,0,0))
