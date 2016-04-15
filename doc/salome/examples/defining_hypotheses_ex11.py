# Projection 1D2D

# Project triangles from one meshed face to another mesh on the same box

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Prepare geometry

# Create a box
box = geompy.MakeBoxDXDYDZ(100, 100, 100)

# Get geom faces to mesh with triangles in the 1ts and 2nd meshes
faces = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
# 2 adjacent faces of the box
Face_1 = faces[2]
Face_2 = faces[0]

geompy.addToStudy( box, 'box' )
geompy.addToStudyInFather( box, Face_1, 'Face_1' )
geompy.addToStudyInFather( box, Face_2, 'Face_2' )


# Make the source mesh triangulated by MEFISTO
src_mesh = smesh.Mesh(Face_1, "Source mesh")
src_mesh.Segment().NumberOfSegments(15)
src_mesh.Triangle()
src_mesh.Compute()

# Mesh the target mesh using the algoritm Projection1D2D
tgt_mesh = smesh.Mesh(Face_2, "Target mesh")
tgt_mesh.Projection1D2D().SourceFace(Face_1,src_mesh)
tgt_mesh.Compute()
