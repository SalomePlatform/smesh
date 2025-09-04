# Viscous layers construction

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

X = geom_builder.MakeVectorDXDYDZ( 1,0,0 )
O = geom_builder.MakeVertex( 100,50,50 )
plane = geom_builder.MakePlane( O, X, 200 ) # plane YZ

box = geom_builder.MakeBoxDXDYDZ(200,100,100)

shape = geom_builder.MakeHalfPartition( box, plane )

faces = geom_builder.SubShapeAllSorted(shape, geom_builder.ShapeType["FACE"])
face1 = faces[1]
# 4 left, 34 middle, 50 right
# Have to pass the middle face id, otherwise it is going to create two disjoint boxes
# because the common face is not going to be ignored and both boxes are going to shrink
# in this direction too
ignoreFaces = [4,34,50]

geom_builder.addToStudy( shape, "shape" )
geom_builder.addToStudyInFather( shape, face1, "face1")

# 3D Viscous layers
mesh = smesh_builder.Mesh(shape, "CFD")

ViscousBuilder = mesh.ViscousLayerBuilder()
thickness = 20	
numberOfLayers = 10
stretchFactor = 1.5
groupName = "Boundary layers"
ViscousBuilder.setBuilderParameters( thickness, numberOfLayers, stretchFactor, 
                                                ignoreFaces,                # optional
                                                groupName = groupName )     # optional

Shrinkshape = ViscousBuilder.GetShrinkGeometry()

shrinkMesh = smesh_builder.Mesh(Shrinkshape, "Shrink")
shrinkMesh.Segment().NumberOfSegments( 4 )
faces = geom_builder.SubShapeAllSorted(Shrinkshape, geom_builder.ShapeType["FACE"])
shrinkFace1 = faces[1]

shrinkMesh.Triangle()
shrinkMesh.Quadrangle(shrinkFace1)
algo3D = shrinkMesh.Tetrahedron()

if not shrinkMesh.Compute(): raise Exception("Error when computing Mesh")

#Add viscous layer
FinalMesh = ViscousBuilder.AddLayers( shrinkMesh )

mesh.MakeGroup("Tetras",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_TETRA)
mesh.MakeGroup("Pyras",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_PYRAMID)
mesh.MakeGroup("Prims",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_PENTA)

# 2D Viscous layers

# 3 edges of the 4 edges of face1
edgeIds = geom_builder.SubShapeAllIDs( face1, geom_builder.ShapeType["EDGE"])[:-1]

mesh = smesh_builder.Mesh(face1,"Face1")
ViscousBuilder = mesh.ViscousLayerBuilder()
ViscousBuilder.setBuilderParameters( 2, 3, 1.5, 
                                    edgeIds, True, # optional
                                    groupName = groupName )     # optional

#For 2D, edges are not selectable (to be developed in occt) the entire face is shrink
shrinkFace = ViscousBuilder.GetShrinkGeometry()
shrinkMesh = smesh_builder.Mesh(shrinkFace, "VicsousLayers2D")

shrinkMesh.Segment().NumberOfSegments( 5 )
algo2D = shrinkMesh.Triangle()

if not shrinkMesh.Compute(): raise Exception("Error when computing Mesh of shrink face")

FinalMeshFace = ViscousBuilder.AddLayers( shrinkMesh )