# Viscous layers construction

from smesh import *
SetCurrentStudy(salome.myStudy)

X = geompy.MakeVectorDXDYDZ( 1,0,0 )
O = geompy.MakeVertex( 100,50,50 )
plane = geompy.MakePlane( O, X, 200 ) # plane YZ

box = geompy.MakeBoxDXDYDZ(200,100,100)

shape = geompy.MakeHalfPartition( box, plane )

faces = geompy.SubShapeAllSorted(shape, geompy.ShapeType["FACE"])
face1 = faces[1]
ignoreFaces = [ faces[0], faces[-1]]

geompy.addToStudy( shape, "shape" )
geompy.addToStudyInFather( shape, face1, "face1")


mesh = Mesh(shape, "CFD")

mesh.Segment().NumberOfSegments( 4 )

mesh.Triangle()
mesh.Quadrangle(face1)
mesh.Compute()
algo3D = mesh.Tetrahedron()

thickness = 20
numberOfLayers = 10
stretchFactor = 1.5
layersHyp = algo3D.ViscousLayers(thickness,numberOfLayers,stretchFactor,ignoreFaces)

mesh.Compute()

mesh.MakeGroup("Tetras",VOLUME,FT_ElemGeomType,"=",Geom_TETRA)
mesh.MakeGroup("Pyras",VOLUME,FT_ElemGeomType,"=",Geom_PYRAMID)
mesh.MakeGroup("Prims",VOLUME,FT_ElemGeomType,"=",Geom_PENTA)
