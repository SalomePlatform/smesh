# Reorient faces

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a geometry consisting of two faces
box = geom_builder.MakeBoxDXDYDZ( 10, 10, 10 )
faces = geom_builder.SubShapeAllSorted( box, geom_builder.ShapeType["FACE"])

shape = geom_builder.MakeCompound( faces[:2] )
faces = geom_builder.SubShapeAll( shape, geom_builder.ShapeType["FACE"] )
geom_builder.addToStudy( shape, "shape")
geom_builder.addToStudyInFather( shape, faces[0], "faces[0]")
geom_builder.addToStudyInFather( shape, faces[1], "faces[1]")

# create a 2D mesh
mesh = smesh_builder.Mesh( shape, "test_Reorient2D")
mesh.AutomaticHexahedralization(0.5)
localAlgo = mesh.Segment(faces[0])
localAlgo.NumberOfSegments( 11 )
if not mesh.Compute(): raise Exception("Error when computing Mesh")
group = mesh.Group( faces[1] )

vec = geom_builder.MakeVectorDXDYDZ( 1, 1, 1 )

# ============
# Reorient2D()
# ============

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
mesh.Reorient2D( localAlgo.GetSubMesh(), [ 1, -1, 1 ], geom_builder.GetFirstVertex( vec ))
#
# 2DObject    - a group of faces
# Direction   - a SMESH.DirStruct structure
# FaceOrPoint - coordinates of a point
mesh.Reorient2D( group, smesh_builder.MakeDirStruct( -10, 1, 10 ), [0,0,0])
#
# FaceOrPoint - a SMESH.PointStruct structure
mesh.Reorient2D( localAlgo.GetSubMesh().GetIDs(), [10,1,0], SMESH.PointStruct(0,0,0))

# ========================
# Reorient2DByNeighbours()
# ========================

# Use faces of 'group' as a reference to reorient equally all faces
mesh.Reorient2DByNeighbours([mesh], [group])

# Orient equally face on 'group', but not define which orientation is correct
mesh.Reorient2DByNeighbours([group])

# =================
# Reorient2DBy3D()
# =================

# Use Reorient2DBy3D() to orient faces of 2 geom faces to have their normal pointing inside volumes

mesh3D = smesh_builder.Mesh( box, '3D mesh')
mesh3D.AutomaticHexahedralization(0.5)
group0 = mesh3D.Group( faces[0] )
group1 = mesh3D.Group( faces[1] )

# pass group0 and ids of faces of group1 to inverse
nbRev = mesh3D.Reorient2DBy3D([ group0, group1.GetIDs() ], mesh3D, theOutsideNormal=False)
print("Nb reoriented faces:", nbRev)

# orient the reversed faces back
nbRev = mesh3D.Reorient2DBy3D( mesh3D, mesh3D, theOutsideNormal=True)
print("Nb re-reoriented faces:", nbRev)
