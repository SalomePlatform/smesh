# Building a compound of meshes

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

## create a bottom box
Box_inf = geompy.MakeBox(0., 0., 0., 200., 200., 50.)

# get a top face
Psup1=geompy.MakeVertex(100., 100., 50.)
Fsup1=geompy.GetFaceNearPoint(Box_inf, Psup1)
# get a bottom face
Pinf1=geompy.MakeVertex(100., 100., 0.)
Finf1=geompy.GetFaceNearPoint(Box_inf, Pinf1)

## create a top box
Box_sup = geompy.MakeBox(100., 100., 50., 200., 200., 100.)

# get a top face
Psup2=geompy.MakeVertex(150., 150., 100.)
Fsup2=geompy.GetFaceNearPoint(Box_sup, Psup2)
# get a bottom face
Pinf2=geompy.MakeVertex(150., 150., 50.)
Finf2=geompy.GetFaceNearPoint(Box_sup, Pinf2)

## Publish in the study
geompy.addToStudy(Box_inf, "Box_inf")
geompy.addToStudyInFather(Box_inf, Fsup1, "Fsup")
geompy.addToStudyInFather(Box_inf, Finf1, "Finf")

geompy.addToStudy(Box_sup, "Box_sup")
geompy.addToStudyInFather(Box_sup, Fsup2, "Fsup")
geompy.addToStudyInFather(Box_sup, Finf2, "Finf")

smesh.SetCurrentStudy(salome.myStudy)

## create a bottom mesh
Mesh_inf = smesh.Mesh(Box_inf, "Mesh_inf")
algo1D_1=Mesh_inf.Segment()
algo1D_1.NumberOfSegments(10)
algo2D_1=Mesh_inf.Quadrangle()
algo3D_1=Mesh_inf.Hexahedron()
Mesh_inf.Compute()

# create a group on the top face
Gsup1=Mesh_inf.Group(Fsup1, "Sup")
# create a group on the bottom face
Ginf1=Mesh_inf.Group(Finf1, "Inf")

## create a top mesh
Mesh_sup = smesh.Mesh(Box_sup, "Mesh_sup")
algo1D_2=Mesh_sup.Segment()
algo1D_2.NumberOfSegments(5)
algo2D_2=Mesh_sup.Quadrangle()
algo3D_2=Mesh_sup.Hexahedron()
Mesh_sup.Compute()

# create a group on the top face
Gsup2=Mesh_sup.Group(Fsup2, "Sup")
# create a group on the bottom face
Ginf2=Mesh_sup.Group(Finf2, "Inf")

## create compounds
# create a compound of two meshes with renaming namesake groups and
# merging of elements with the given tolerance
Compound1 = smesh.Concatenate([Mesh_inf, Mesh_sup], 0, 1, 1e-05,
                              name='Compound_with_RenamedGrps_and_MergeElems')
# create a compound of two meshes with uniting namesake groups and
# creating groups of all elements
Compound2 = smesh.Concatenate([Mesh_inf, Mesh_sup], 1, 0, 1e-05, True,
                              name='Compound_with_UniteGrps_and_GrpsOfAllElems')

if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(True)
