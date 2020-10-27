# "Elements of a domain" filter and "Renumber" hypothesis

import salome, SMESH
salome.salome_init()
from salome.geom import geomBuilder
geompy = geomBuilder.New()
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

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
ijkAlgo = mesh.Hexahedron()

# Use Renumber hypothesis to get hexahedra and nodes numbered like in a structured mesh.
# k axis of box1 will be ( 100,100,0 ) - ( 100,100,100 )
# k axis of box2 will be ( 0,0,0 ) - (0,0,100), by default
v000 = geompy.MakeVertex( 100,100,0, theName='v000' ) # can use box sub-vertex or standalone one
v001 = geompy.GetVertexNearPoint( box1, geompy.MakeVertex(100,100,100), theName='v001')
ijkAlgo.Renumber([ smeshBuilder.BlockCS( box1, v000, v001 ) ])

mesh.Compute()

# Create filters with FT_ConnectedElements criterion by pointing a domain in different ways:

# using point coordinates in box_1
nodeFilter = smesh.GetFilter( SMESH.NODE, SMESH.FT_ConnectedElements, "=", "1.,2,10", mesh=mesh )
print("Nb. nodes in box_1:", len( nodeFilter.GetIDs()))

# using point coordinates in box_2
edgeFilter = smesh.GetFilter( SMESH.EDGE, SMESH.FT_ConnectedElements, "=", [202,1,1 ], mesh=mesh )
print("Nb. segments in box_2:", len( edgeFilter.GetIDs()))

# using a geom vertex of box_1
faceFilter = smesh.GetFilter( SMESH.FACE, SMESH.FT_ConnectedElements, "=", vertex, mesh=mesh )
print("Nb. faces in box_1:", len( edgeFilter.GetIDs()))

# using node ID in box_2
voluFilter = smesh.GetFilter( SMESH.VOLUME, SMESH.FT_ConnectedElements, "=", 10, mesh=mesh )
print("Nb. volumes in box_2:", len( voluFilter.GetIDs()))

