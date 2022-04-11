# "Import 2D Elements from Another Mesh" example

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Make a patritioned box

box = geom_builder.MakeBoxDXDYDZ(100,100,100)

N = geom_builder.MakeVectorDXDYDZ( 1,0,0 )
O = geom_builder.MakeVertex( 50,0,0 )
plane = geom_builder.MakePlane( O, N, 200 ) # plane YOZ

shape2boxes = geom_builder.MakeHalfPartition( box, plane )
boxes = geom_builder.SubShapeAllSorted(shape2boxes, geom_builder.ShapeType["SOLID"])

geom_builder.addToStudy( boxes[0], "boxes[0]")
geom_builder.addToStudy( boxes[1], "boxes[1]")
midFace0 = geom_builder.SubShapeAllSorted(boxes[0], geom_builder.ShapeType["FACE"])[5]
geom_builder.addToStudyInFather( boxes[0], midFace0, "middle Face")
midFace1 = geom_builder.SubShapeAllSorted(boxes[1], geom_builder.ShapeType["FACE"])[0]
geom_builder.addToStudyInFather( boxes[1], midFace1, "middle Face")

# Mesh one of boxes with quadrangles. It is a source mesh

srcMesh = smesh_builder.Mesh(boxes[0], "source mesh") # box coloser to CS origin
nSeg1 = srcMesh.Segment().NumberOfSegments(4)
srcMesh.Quadrangle()
srcMesh.Compute()
srcFaceGroup = srcMesh.GroupOnGeom( midFace0, "src faces", SMESH.FACE )

# Import faces from midFace0 to the target mesh

tgtMesh = smesh_builder.Mesh(boxes[1], "target mesh")
importAlgo = tgtMesh.UseExisting2DElements(midFace1)
import2hyp = importAlgo.SourceFaces( [srcFaceGroup] )
tgtMesh.Segment().NumberOfSegments(3)
tgtMesh.Quadrangle()
tgtMesh.Compute()

# Import the whole source mesh with groups
import2hyp.SetCopySourceMesh(True,True)
tgtMesh.Compute()
