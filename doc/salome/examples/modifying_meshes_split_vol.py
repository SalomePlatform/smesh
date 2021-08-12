# Split volumic elements into tetrahedrons

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
geompy = geomBuilder.New()
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New()

# mesh a hexahedral mesh
box = geompy.MakeBoxDXDYDZ (1, 1, 1 )
mesh = smesh.Mesh( box )
mesh.AutomaticHexahedralization(0)
print("Nb volumes mesh: %s" % mesh.NbHexas())

# split each hexahedron into 6 tetrahedra
mesh.SplitVolumesIntoTetra( mesh, smesh.Hex_6Tet )
print("Nb volumes mesh: %s" % mesh.NbTetras())
