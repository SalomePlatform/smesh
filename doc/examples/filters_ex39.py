# "Elements of a domain" filter and "Renumber" hypothesis

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create two boxes to have two domains in the mesh

box1 = geom_builder.MakeBoxDXDYDZ( 100,100,100 )
box2 = geom_builder.MakeTranslation( box1, 200, 0, 0 )
boxes = geom_builder.MakeCompound( [box1, box2] )
box1, box2 = geom_builder.SubShapeAll( boxes, geom_builder.ShapeType["SHAPE"], "box")

vertex = geom_builder.SubShape( box1, geom_builder.ShapeType["VERTEX"], [1] )

# create a mesh

mesh = smesh_builder.Mesh( boxes )
mesh.Segment(box1).NumberOfSegments( 5 )  # to have different nb of elements on the boxes
mesh.Segment(box2).NumberOfSegments( 10 )
mesh.Quadrangle()
ijkAlgo = mesh.Hexahedron()

# Use Renumber hypothesis to get hexahedra and nodes numbered like in a structured mesh.
# k axis of box1 will be ( 100,100,0 ) - ( 100,100,100 )
# k axis of box2 will be ( 0,0,0 ) - (0,0,100), by default
v000 = geom_builder.MakeVertex( 100,100,0, theName='v000' ) # can use box sub-vertex or standalone one
v001 = geom_builder.GetVertexNearPoint( box1, geom_builder.MakeVertex(100,100,100), theName='v001')
ijkAlgo.Renumber([ smeshBuilder.BlockCS( box1, v000, v001 ) ])

mesh.Compute()

# Create filters with FT_ConnectedElements criterion by pointing a domain in different ways:

# using point coordinates in box_1
nodeFilter = smesh_builder.GetFilter( SMESH.NODE, SMESH.FT_ConnectedElements, "=", "1.,2,10", mesh=mesh )
print("Nb. nodes in box_1:", len( nodeFilter.GetIDs()))

# using point coordinates in box_2
edgeFilter = smesh_builder.GetFilter( SMESH.EDGE, SMESH.FT_ConnectedElements, "=", [202,1,1 ], mesh=mesh )
print("Nb. segments in box_2:", len( edgeFilter.GetIDs()))

# using a geom vertex of box_1
faceFilter = smesh_builder.GetFilter( SMESH.FACE, SMESH.FT_ConnectedElements, "=", vertex, mesh=mesh )
print("Nb. faces in box_1:", len( edgeFilter.GetIDs()))

# using node ID in box_2
voluFilter = smesh_builder.GetFilter( SMESH.VOLUME, SMESH.FT_ConnectedElements, "=", 10, mesh=mesh )
print("Nb. volumes in box_2:", len( voluFilter.GetIDs()))
