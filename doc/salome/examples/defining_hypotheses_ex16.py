# "Import 2D Elements from Another Mesh" example

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Make a patritioned box

box = geompy.MakeBoxDXDYDZ(100,100,100)

N = geompy.MakeVectorDXDYDZ( 1,0,0 )
O = geompy.MakeVertex( 50,0,0 )
plane = geompy.MakePlane( O, N, 200 ) # plane YOZ

shape2boxes = geompy.MakeHalfPartition( box, plane )
boxes = geompy.SubShapeAllSorted(shape2boxes, geompy.ShapeType["SOLID"])

geompy.addToStudy( boxes[0], "boxes[0]")
geompy.addToStudy( boxes[1], "boxes[1]")
midFace0 = geompy.SubShapeAllSorted(boxes[0], geompy.ShapeType["FACE"])[5]
geompy.addToStudyInFather( boxes[0], midFace0, "middle Face")
midFace1 = geompy.SubShapeAllSorted(boxes[1], geompy.ShapeType["FACE"])[0]
geompy.addToStudyInFather( boxes[1], midFace1, "middle Face")

# Mesh one of boxes with quadrangles. It is a source mesh

srcMesh = smesh.Mesh(boxes[0], "source mesh") # box coloser to CS origin
nSeg1 = srcMesh.Segment().NumberOfSegments(4)
srcMesh.Quadrangle()
srcMesh.Compute()
srcFaceGroup = srcMesh.GroupOnGeom( midFace0, "src faces", SMESH.FACE )

# Import faces from midFace0 to the target mesh

tgtMesh = smesh.Mesh(boxes[1], "target mesh")
importAlgo = tgtMesh.UseExisting2DElements(midFace1)
import2hyp = importAlgo.SourceFaces( [srcFaceGroup] )
tgtMesh.Segment().NumberOfSegments(3)
tgtMesh.Quadrangle()
tgtMesh.Compute()

# Import the whole source mesh with groups
import2hyp.SetCopySourceMesh(True,True)
tgtMesh.Compute()
