# Elements of a domain

import salome, SMESH
salome.salome_init()
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create two boxes to have two domains in the mesh

box1 = geompy.MakeBoxDXDYDZ( 100,100,100 )
box2 = geompy.MakeTranslation( box1, 200, 0, 0 )
boxes = geompy.MakeCompound( [box1, box2] )
box1, box2 = geompy.SubShapeAll( boxes, geompy.ShapeType["SHAPE"], "box")

vertex = geompy.SubShape( box1, geompy.ShapeType["VERTEX"], [1] )

# create a mesh

mesh = smesh.Mesh( boxes )
mesh.Segment(box1).NumberOfSegments( 5 )  # to have different nb of elements on the boxes
mesh.Segment(box2).NumberOfSegments( 10 )
mesh.Quadrangle()
mesh.Hexahedron()
mesh.Compute()

# Create filters with FT_ConnectedElements criterion by pointing a domain in different ways:

# using point coordinates in box_1
nodeFilter = smesh.GetFilter( SMESH.NODE, SMESH.FT_ConnectedElements, "=", "1.,2,10", mesh=mesh )
print "Nb. nodes in box_1:", len( nodeFilter.GetIDs())

# using point coordinates in box_2
edgeFilter = smesh.GetFilter( SMESH.EDGE, SMESH.FT_ConnectedElements, "=", [202,1,1 ], mesh=mesh )
print "Nb. segments in box_2:", len( edgeFilter.GetIDs())

# using a geom vertex of box_1
faceFilter = smesh.GetFilter( SMESH.FACE, SMESH.FT_ConnectedElements, "=", vertex, mesh=mesh )
print "Nb. faces in box_1:", len( edgeFilter.GetIDs())

# using node ID in box_2
voluFilter = smesh.GetFilter( SMESH.VOLUME, SMESH.FT_ConnectedElements, "=", 10, mesh=mesh )
print "Nb. volumes in box_2:", len( voluFilter.GetIDs())

