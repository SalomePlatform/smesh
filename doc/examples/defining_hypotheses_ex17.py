# Viscous layers construction

import salome
salome.salome_init_without_session()

import SMESH
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
ignoreFaces = [ faces[0], faces[-1]]

geom_builder.addToStudy( shape, "shape" )
geom_builder.addToStudyInFather( shape, face1, "face1")

# 3D Viscous layers

mesh = smesh_builder.Mesh(shape, "CFD")

mesh.Segment().NumberOfSegments( 4 )

mesh.Triangle()
mesh.Quadrangle(face1)
algo3D = mesh.Tetrahedron()

thickness = 20
numberOfLayers = 10
stretchFactor = 1.5
groupName = "Boundary layers"
layersHyp = algo3D.ViscousLayers(thickness,numberOfLayers,stretchFactor,
                                 ignoreFaces,           # optional
                                 groupName = groupName) # optional

if not mesh.Compute(): raise Exception("Error when computing Mesh")

# retrieve boundary prisms created by mesh.Compute()
boundaryGroup = mesh.GetGroupByName( layersHyp.GetGroupName() )[0]
print( "Nb boundary prisms", boundaryGroup.Size() )

mesh.MakeGroup("Tetras",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_TETRA)
mesh.MakeGroup("Pyras",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_PYRAMID)
mesh.MakeGroup("Prims",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_PENTA)

# 2D Viscous layers

# 3 edges of the 4 edges of face1
edgeIds = geom_builder.SubShapeAllIDs( face1, geom_builder.ShapeType["EDGE"])[:-1]

mesh = smesh_builder.Mesh(face1,"VicsousLayers2D")
mesh.Segment().NumberOfSegments( 5 )

# viscous layers will be created on 1 edge, as we set 3 edges to ignore
vlHyp = mesh.Triangle().ViscousLayers2D( 2, 3, 1.5,
                                         edgeIds, isEdgesToIgnore=True, # optional
                                         groupName=groupName)           # optional
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# retrieve boundary elements created by mesh.Compute()
quadrangles = mesh.GetGroupByName( vlHyp.GetGroupName() )[0]
print( "Nb boundary quadrangles", quadrangles.Size() )

# viscous layers will be created on 3 edges, as we pass isEdgesToIgnore=False
vlHyp.SetEdges( edgeIds, False )

if not mesh.Compute(): raise Exception("Error when computing Mesh")
