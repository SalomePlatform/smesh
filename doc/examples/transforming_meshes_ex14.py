# Create 2D mesh from 3D elements

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

box = geom_builder.MakeBoxDXDYDZ(100, 100, 100)

gFaces = geom_builder.SubShapeAllSorted(box, geom_builder.ShapeType["FACE"])
f1,f2 = gFaces[0],gFaces[1]
geom_builder.addToStudy(box,"box")
geom_builder.addToStudyInFather(box,f1,"face1")
geom_builder.addToStudyInFather(box,f2,"face2")

twoFaces = geom_builder.MakeCompound([f1,f2])

## -----------
##
## Create 2D from 3D elements
##
## -----------

init_mesh = smesh_builder.Mesh(box, "box")
init_mesh.AutomaticHexahedralization() # it makes 3 x 3 x 3 hexahedrons
init_mesh.Compute()

mesh_1 = smesh_builder.CopyMesh(init_mesh, "Mesh_1")
#Return the number of created faces in the original mesh.
nb, new_mesh, new_group = mesh_1.MakeBoundaryOfEachElement()

if nb != 54: raise Exception("Error on MakeBoundaryOfEachElement call. The number of created elements does not match.")
if new_mesh.GetId() != mesh_1.GetId(): raise Exception("The mesh created from MakeBoundaryElements should be the same of the call")
if new_group: raise Exception("The group created from MakeBoundaryElements should be undefined")

mesh_2 = smesh_builder.CopyMesh(init_mesh, "Mesh_2")

#Return the number of created faces and a new_mesh and new_group 
nb, new_mesh, new_group = mesh_2.MakeBoundaryOfEachElement("MyFacesElements", "Face_Mesh")
if nb != 54: raise Exception("Error on MakeBoundaryOfEachElement call. The number of created elements does not match.")
if new_mesh.GetId() == mesh_2.GetId(): raise Exception("The mesh created from MakeBoundaryElements should be the different of the call")
if not new_group: raise Exception("The group created from MakeBoundaryElements should be defined")
