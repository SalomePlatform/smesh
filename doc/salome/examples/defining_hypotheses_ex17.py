# Viscous layers construction

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

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


mesh = smesh.Mesh(shape, "CFD")

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

mesh.MakeGroup("Tetras",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_TETRA)
mesh.MakeGroup("Pyras",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_PYRAMID)
mesh.MakeGroup("Prims",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_PENTA)
