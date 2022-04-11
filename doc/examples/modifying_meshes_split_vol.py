# Split volumic elements into tetrahedrons

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# mesh a hexahedral mesh
box = geom_builder.MakeBoxDXDYDZ (1, 1, 1 )
mesh = smesh_builder.Mesh( box )
mesh.AutomaticHexahedralization(0)
print("Nb volumes mesh: %s" % mesh.NbHexas())

# split each hexahedron into 6 tetrahedra
mesh.SplitVolumesIntoTetra( mesh, smesh_builder.Hex_6Tet )
print("Nb volumes mesh: %s" % mesh.NbTetras())
