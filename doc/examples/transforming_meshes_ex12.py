# Create boundary elements

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
## 2D from 3D
##
## -----------
dim = SMESH.BND_2DFROM3D

init_mesh = smesh_builder.Mesh(box, "box")
init_mesh.AutomaticHexahedralization() # it makes 3 x 3 x 3 hexahedrons

# remove some faces
faces = init_mesh.GetElementsByType( SMESH.FACE )
nb_faces = len( faces )
rm_face = faces[ : nb_faces // 2]
init_mesh.RemoveElements( rm_face )

# restore boundary in this mesh
mesh = smesh_builder.CopyMesh( init_mesh, "2D from 3D")
groupName = "bnd 2D"
nb, new_mesh, new_group = mesh.MakeBoundaryElements(dim, groupName)

# restore boundary (only) in other mesh
meshName = "2D boundary of " + init_mesh.GetName()
nb, new_mesh, new_group = init_mesh.MakeBoundaryElements(dim, groupName, meshName)

# restore boundary in mesh copy
meshName = init_mesh.GetName() + " + boundary"
nb, new_mesh, new_group = init_mesh.MakeBoundaryElements(dim, groupName, meshName, toCopyAll=True)

## -----------
##
## 1D from 2D
##
## -----------
dim = SMESH.BND_1DFROM2D

init_mesh = smesh_builder.Mesh(f1, "2D mesh")
init_mesh.AutomaticHexahedralization()

# remove some edges
edges = init_mesh.GetElementsByType( SMESH.EDGE )
nb_edges = len( edges )
rm_edge = edges[ : nb_edges // 2]
init_mesh.RemoveElements( rm_edge )


# restore boundary edges in this mesh
mesh = smesh_builder.CopyMesh( init_mesh, "1D from 2D")
groupName = "bnd 1D"
nb, new_mesh, new_group = mesh.MakeBoundaryElements(dim, groupName)

# restore boundary edges (only) in other mesh
meshName = "1D boundary of " + init_mesh.GetName()
nb, new_mesh, new_group = init_mesh.MakeBoundaryElements(dim, groupName, meshName)

# restore boundary edges in mesh copy
meshName = init_mesh.GetName() + " + boundary"
nb, new_mesh, new_group = init_mesh.MakeBoundaryElements(dim, groupName, meshName, toCopyAll=True)

## ------------------
##
## 1D from 2D GROUPS
##
## ------------------
dim = SMESH.BND_1DFROM3D

init_mesh = smesh_builder.Mesh(box, "box")
init_mesh.AutomaticHexahedralization() # it makes 3 x 3 x 3 hexahedrons
# remove all edges
rm_edges = init_mesh.GetElementsByType( SMESH.EDGE )
init_mesh.RemoveElements( rm_edges )

# make groups of faces
fGroup1 = init_mesh.Group( f1, "f1" )
fGroup2 = init_mesh.Group( f2, "f2" )

# make 1D boundary around groups in this mesh
mesh = smesh_builder.CopyMesh( init_mesh, "1D from 2D groups", toCopyGroups=True)
groups = mesh.GetGroups()
nb, new_mesh, new_group = mesh.MakeBoundaryElements(dim, groupName,groups=groups)

# make 1D boundary (only) in other mesh
meshName =  "boundary from groups of " + init_mesh.GetName()
groups = init_mesh.GetGroups()
nb, new_mesh, new_group = init_mesh.MakeBoundaryElements(dim, groupName, meshName,groups=groups)

# make 1D boundary in mesh copy
meshName = init_mesh.GetName() + " + boundary from groups"
nb, new_mesh, new_group = init_mesh.MakeBoundaryElements(dim, groupName, meshName,
                                                         groups=groups, toCopyAll=True)

