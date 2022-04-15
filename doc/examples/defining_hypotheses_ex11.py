# Projection 1D2D
# Project triangles from one meshed face to another mesh on the same box

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Prepare geometry

# Create a box
box = geom_builder.MakeBoxDXDYDZ(100, 100, 100)

# Get geom faces to mesh with triangles in the 1ts and 2nd meshes
faces = geom_builder.SubShapeAll(box, geom_builder.ShapeType["FACE"])
# 2 adjacent faces of the box
Face_1 = faces[2]
Face_2 = faces[0]

geom_builder.addToStudy( box, 'box' )
geom_builder.addToStudyInFather( box, Face_1, 'Face_1' )
geom_builder.addToStudyInFather( box, Face_2, 'Face_2' )

# Make the source mesh triangulated by default algorithm
src_mesh = smesh_builder.Mesh(Face_1, "Source mesh")
src_mesh.Segment().NumberOfSegments(15)
src_mesh.Triangle()
src_mesh.Compute()

# Mesh the target mesh using the algorithm Projection1D2D
tgt_mesh = smesh_builder.Mesh(Face_2, "Target mesh")
tgt_mesh.Projection1D2D().SourceFace(Face_1,src_mesh)
tgt_mesh.Compute()
