# Projection Algorithms

# Project prisms from one meshed box to another mesh on the same box

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Prepare geometry

# Create a parallelepiped
box = geom_builder.MakeBoxDXDYDZ(200, 100, 70)
geom_builder.addToStudy( box, "box" )

# Get geom faces to mesh with triangles in the 1ts and 2nd meshes
faces = geom_builder.SubShapeAll(box, geom_builder.ShapeType["FACE"])
# 2 adjacent faces of the box
f1 = faces[2]
f2 = faces[0]
# face opposite to f2
f2opp = geom_builder.GetOppositeFace( box, f2 )

# Get vertices used to specify how to associate sides of faces at projection
[v1F1, v2F1] = geom_builder.SubShapeAll(f1, geom_builder.ShapeType["VERTEX"])[:2]
[v1F2, v2F2] = geom_builder.SubShapeAll(f2, geom_builder.ShapeType["VERTEX"])[:2]
geom_builder.addToStudyInFather( box, v1F1, "v1F1" )
geom_builder.addToStudyInFather( box, v2F1, "v2F1" )
geom_builder.addToStudyInFather( box, v1F2, "v1F2" )
geom_builder.addToStudyInFather( box, v2F2, "v2F2" )

# Make group of 3 edges of f1 and f2
edgesF1 = geom_builder.CreateGroup(f1, geom_builder.ShapeType["EDGE"])
geom_builder.UnionList( edgesF1, geom_builder.SubShapeAll(f1, geom_builder.ShapeType["EDGE"])[:3])
edgesF2 = geom_builder.CreateGroup(f2, geom_builder.ShapeType["EDGE"])
geom_builder.UnionList( edgesF2, geom_builder.SubShapeAll(f2, geom_builder.ShapeType["EDGE"])[:3])
geom_builder.addToStudyInFather( box, edgesF1, "edgesF1" )
geom_builder.addToStudyInFather( box, edgesF2, "edgesF2" )

# Make the source mesh with prisms
src_mesh = smesh_builder.Mesh(box, "Source mesh")
src_mesh.Segment().NumberOfSegments(9,10)
src_mesh.Quadrangle()
src_mesh.Hexahedron()
src_mesh.Triangle(f1) # triangular sub-mesh
if not src_mesh.Compute(): raise Exception("Error when computing Mesh")

# Mesh the box using projection algorithms

# Define the same global 1D and 2D hypotheses
tgt_mesh = smesh_builder.Mesh(box, "Target mesh")
tgt_mesh.Segment().NumberOfSegments(9,10,UseExisting=True)
tgt_mesh.Quadrangle()

# Define Projection 1D algorithm to project 1d mesh elements from group edgesF2 to edgesF1
# It is actually not needed, just a demonstration
proj1D = tgt_mesh.Projection1D( edgesF1 )
# each vertex must be at the end of a connected group of edges (or a sole edge)
proj1D.SourceEdge( edgesF2, src_mesh, v2F1, v2F2 )

# Define 2D hypotheses to project triangles from f1 face of the source mesh to
# f2 face in the target mesh. Vertices specify how to associate sides of faces
proj2D = tgt_mesh.Projection2D( f2 )
proj2D.SourceFace( f1, src_mesh, v1F1, v1F2, v2F1, v2F2 )

# 2D hypotheses to project triangles from f2 of target mesh to the face opposite to f2.
# Association of face sides is default
proj2D = tgt_mesh.Projection2D( f2opp )
proj2D.SourceFace( f2 )

# 3D hypotheses to project prisms from the source to the target mesh
proj3D = tgt_mesh.Projection3D()
proj3D.SourceShape3D( box, src_mesh, v1F1, v1F2, v2F1, v2F2 )
if not tgt_mesh.Compute(): raise Exception("Error when computing Mesh")

# Move the source mesh to visually compare the two meshes
src_mesh.TranslateObject( src_mesh, smesh_builder.MakeDirStruct( 210, 0, 0 ), Copy=False)
