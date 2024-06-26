# Building a compound of meshes

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

## create a bottom box
Box_inf = geom_builder.MakeBox(0., 0., 0., 200., 200., 50.)

# get a top face
Psup1=geom_builder.MakeVertex(100., 100., 50.)
Fsup1=geom_builder.GetFaceNearPoint(Box_inf, Psup1)
# get a bottom face
Pinf1=geom_builder.MakeVertex(100., 100., 0.)
Finf1=geom_builder.GetFaceNearPoint(Box_inf, Pinf1)

## create a top box
Box_sup = geom_builder.MakeBox(100., 100., 50., 200., 200., 100.)

# get a top face
Psup2=geom_builder.MakeVertex(150., 150., 100.)
Fsup2=geom_builder.GetFaceNearPoint(Box_sup, Psup2)
# get a bottom face
Pinf2=geom_builder.MakeVertex(150., 150., 50.)
Finf2=geom_builder.GetFaceNearPoint(Box_sup, Pinf2)

## Publish in the study
geom_builder.addToStudy(Box_inf, "Box_inf")
geom_builder.addToStudyInFather(Box_inf, Fsup1, "Fsup")
geom_builder.addToStudyInFather(Box_inf, Finf1, "Finf")

geom_builder.addToStudy(Box_sup, "Box_sup")
geom_builder.addToStudyInFather(Box_sup, Fsup2, "Fsup")
geom_builder.addToStudyInFather(Box_sup, Finf2, "Finf")

smesh_builder.UpdateStudy()

## create a bottom mesh
Mesh_inf = smesh_builder.Mesh(Box_inf, "Mesh_inf")
algo1D_1=Mesh_inf.Segment()
algo1D_1.NumberOfSegments(10)
algo2D_1=Mesh_inf.Quadrangle()
algo3D_1=Mesh_inf.Hexahedron()
if not Mesh_inf.Compute(): raise Exception("Error when computing Mesh")

# create a group on the top face
Gsup1=Mesh_inf.Group(Fsup1, "Sup")
# create a group on the bottom face
Ginf1=Mesh_inf.Group(Finf1, "Inf")

## create a top mesh
Mesh_sup = smesh_builder.Mesh(Box_sup, "Mesh_sup")
algo1D_2=Mesh_sup.Segment()
algo1D_2.NumberOfSegments(5)
algo2D_2=Mesh_sup.Quadrangle()
algo3D_2=Mesh_sup.Hexahedron()
if not Mesh_sup.Compute(): raise Exception("Error when computing Mesh")

# create a group on the top face
Gsup2=Mesh_sup.Group(Fsup2, "Sup")
# create a group on the bottom face
Ginf2=Mesh_sup.Group(Finf2, "Inf")

## create compounds
# create a compound of two meshes with renaming namesake groups and
# merging elements with the given tolerance
Compound1 = smesh_builder.Concatenate([Mesh_inf, Mesh_sup], 0, 1, 1e-05,
                                      name='Compound with RenamedGrps and MergeElems')
# create a compound of two meshes with uniting namesake groups and
# creating groups of all elements
Compound2 = smesh_builder.Concatenate([Mesh_inf, Mesh_sup], 1, 0, 1e-05, True,
                                      name='Compound with UniteGrps and GrpsOfAllElems')

# copy Gsup1 into a separate mesh and translate it
groupMesh = Mesh_inf.TranslateObjectMakeMesh( Gsup1, [300,0,0] )

# add Ginf2 to groupMesh
smesh_builder.Concatenate([Ginf2], False, meshToAppendTo = groupMesh )

if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser()
