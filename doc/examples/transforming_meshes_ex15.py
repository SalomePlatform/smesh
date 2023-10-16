# Offset 2D mesh, sub-mesh, group or elements from filter.

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

box = geom_builder.MakeBoxDXDYDZ(100, 100, 100)

## -----------
##
## Offset mesh
##
## -----------

init_mesh = smesh_builder.Mesh(box, "box")
init_mesh.AutomaticTetrahedralization()
init_mesh.Compute()

#Offset triangular elements in mesh (expand the entire mesh)
offset = 20
mesh, mesh_group = init_mesh.Offset( init_mesh, offset, MakeGroups=False, CopyElements=False )
assert isinstance( mesh, smeshBuilder.Mesh )
assert len( mesh_group ) == 0
offsetMeshArea = smesh_builder.GetArea(mesh)

#Offset triangular elements in mesh (shrink the entire mesh)
offset = -20
mesh, mesh_group = init_mesh.Offset( init_mesh, offset, MakeGroups=False, CopyElements=False, NewMeshName="MeshShrink" )
assert isinstance( mesh, smeshBuilder.Mesh )
assert len( mesh_group ) == 0
assert mesh.GetName() == "MeshShrink"
shrinkMeshArea = smesh_builder.GetArea(mesh)

assert offsetMeshArea > shrinkMeshArea