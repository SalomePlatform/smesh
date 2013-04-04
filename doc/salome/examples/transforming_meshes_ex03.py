# Scale

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

Box = geompy.MakeBoxDXDYDZ(200, 200, 200)
f = geompy.SubShapeAllSorted(Box, geompy.ShapeType["FACE"])

Mesh1 = smesh.Mesh(f[0])
Regular_1D = Mesh1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(3)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh1.Quadrangle()
isDone = Mesh1.Compute()

#Perform scale opration for the whole mesh and creation of a new mesh
newMesh = Mesh1.ScaleMakeMesh(Mesh1,SMESH.PointStruct(100,100,200),[0.5,0.3,0.7],True,"ScaledMesh")

#Perform scale operation for the whole mesh and copy elements
Mesh1.Scale(Mesh1,SMESH.PointStruct(200,100,100),[0.5,0.5,0.5],True,True)

#Perform scale opration for two edges and move elements
Mesh1.Scale([1,2],SMESH.PointStruct(-100,100,100),[0.8,1.0,0.7],False)

#Perform scale opration for one face and move elements
Mesh1.Scale([21],SMESH.PointStruct(0,200,200),[0.7,0.7,0.7],False)
